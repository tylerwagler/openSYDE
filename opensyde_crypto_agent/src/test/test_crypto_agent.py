# Test application for osy_crypto_agent.
# Uses pytest and subprocess to start the agent binary and send TCP requests to it.
import os
import pathlib
import signal
import socket
import struct
import subprocess
import time
from typing import Any, Optional, Sequence, cast

import pytest

SERVER_IP = "127.0.0.1"
SERVER_PORT = 50963
ALTERNATIVE_SERVER_PORT = 50964

def DoTcpRequest(request: bytearray, timeout_s: float = 2.0, server_ip: str = SERVER_IP,
                 server_port: int = SERVER_PORT):
   with socket.create_connection((server_ip, server_port), timeout=timeout_s) as sock:
      sock.sendall(request)
      response = sock.recv(1024)

      print("Request (hex):", request.hex())
      print("Response (hex):", response.hex())
      return response

def _BuildServiceRequest(service_id: int, payload: bytes = b"") -> bytearray:
   request_data = bytearray()
   request_data.extend(b"\x00\x00\x00\x00")
   request_data.append(service_id & 0xFF)
   request_data.extend(payload)
   size = len(request_data) - 4
   request_data[0:4] = struct.pack("!I", size)
   return request_data

def _AssertResponseHeader(response: bytes, expected_service_id: int, expected_result: int,
                          minimum_payload_size: int = 0):
   assert len(response) >= (4 + 1 + 1 + minimum_payload_size)
   size_header, = struct.unpack(">I", response[0:4])
   assert size_header == (len(response) - 4)
   assert response[4] == expected_service_id
   assert response[5] == expected_result

def _CreateSolveRequest(certificate_serial_number: bytes) -> bytearray:
   payload = bytearray()
   payload.extend(b"\x01")
   payload.extend(b"\x00")
   payload.extend(b"\x05")
   payload.extend(b"\x00\x03")
   payload.extend(b"STW")
   payload.extend(b"\x00\x04")
   payload.extend(b"ESX9")
   payload.extend(b"\x00\x08")
   payload.extend(b"THE_NODE")
   payload.extend(b"\x00")
   payload.extend(b"\x00")
   payload.extend(b"\x00\x06")
   payload.extend(b"\x11\x22\x33\x44\x55\x66")
   payload.extend(b"\x00\x14")
   payload.extend(certificate_serial_number)
   payload.extend(b"\x00\x08")
   payload.extend(b"\xAA\xBB\xCC\xDD\xEE\xFF\x11\x22")
   return _BuildServiceRequest(0x02, payload)

class CryptoAgentHarness:
   def __init__(self, server_ip: str = SERVER_IP, server_port: int = SERVER_PORT):
      self.process: Optional[subprocess.Popen[str]] = None
      self.exit_code = None
      self.binary_path = self.ResolveBinaryPath()
      self.server_ip = server_ip
      self.server_port = server_port

   @staticmethod
   def BuildProcessArgs(binary_path: pathlib.Path, args: Optional[Sequence[str]] = None):
      process_args = [str(binary_path)]
      if args is not None:
         process_args.extend(args)

      # under Linux run using valgrind and check for memory leaks
      # for report use %p to get unique file per process
      if os.name != "nt":
         process_args = [
            "valgrind",
            "--leak-check=yes",
            "--error-exitcode=99",
            "--xml=yes",
            "--xml-file=valgrind_report_%p.xml"
         ] + process_args

      return process_args

   @staticmethod
   def ResolveBinaryPath() -> pathlib.Path:
      repo_root = pathlib.Path(__file__).resolve().parents[2]
      result_dir = repo_root / "result"

      if os.name == "nt":
         candidate = result_dir / "osy_crypto_agent.exe"
      else:
         candidate = result_dir / "osy_crypto_agent"

      if candidate.exists():
         return candidate

      raise FileNotFoundError(
         "Could not find crypto agent binary in ./result"
      )

   def _BuildPopenKwargs(self) -> dict[str, Any]:
      popen_kwargs: dict[str, Any] = {
         "cwd": str(self.binary_path.parent),
         "stdout": subprocess.PIPE,
         "stderr": subprocess.STDOUT,
         "text": True,
      }

      if os.name == "nt":
         # CREATE_NEW_PROCESS_GROUP allows us to send CTRL_BREAK_EVENT to the process group on Windows for graceful shutdown
         popen_kwargs["creationflags"] = subprocess.CREATE_NEW_PROCESS_GROUP
      else:
         # start_new_session allows us to send signals to the process group on Unix for graceful shutdown
         popen_kwargs["start_new_session"] = True

      return popen_kwargs

   def start_raw(self, args: Optional[Sequence[str]] = None):
      if self.process is not None:
         raise RuntimeError("Server already started")

      process_args = self.BuildProcessArgs(self.binary_path, args)
      self.process = subprocess.Popen(process_args, **self._BuildPopenKwargs())

   def start(self, startup_timeout_s: float = 5.0, args: Optional[Sequence[str]] = None):
      self.start_raw(args)
      assert self.process is not None

      deadline = time.time() + startup_timeout_s
      last_error = None
      while time.time() < deadline:
         if self.process.poll() is not None:
            self.exit_code = self.process.returncode
            raise RuntimeError(f"Server exited during startup with code {self.exit_code}")

         try:
            ping_req = _BuildServiceRequest(0x00)
            response = DoTcpRequest(ping_req, timeout_s=0.5, server_ip=self.server_ip, server_port=self.server_port)
            if len(response) >= 6 and response[4] == 0x00 and response[5] == 0x00:
               return
         except Exception as ex:
            last_error = ex
            time.sleep(0.1)

      raise TimeoutError(f"Server did not become ready within {startup_timeout_s}s: {last_error}")

   def _request_shutdown_via_tcp(self):
      try:
         shutdown_req = _BuildServiceRequest(0x01)
         response = DoTcpRequest(shutdown_req, timeout_s=1.0, server_ip=self.server_ip, server_port=self.server_port)
         # Expected: size(4), service id(1), result(1)
         return len(response) >= 6 and response[4] == 0x01 and response[5] == 0x00
      except Exception:
         return False

   def stop(self, graceful_timeout_s: float = 3.0, force_timeout_s: float = 3.0) -> int:
      if self.process is None:
         return self.exit_code if self.exit_code is not None else 0

      if self.process.poll() is None:
         self._request_shutdown_via_tcp()
         try:
            self.exit_code = self.process.wait(timeout=graceful_timeout_s)
         except subprocess.TimeoutExpired:
            if os.name == "nt":
               try:
                  self.process.send_signal(signal.CTRL_BREAK_EVENT)
               except Exception:
                  pass
            else:
               try:
                  os.killpg(self.process.pid, signal.SIGINT)
               except Exception:
                  pass

            try:
               self.exit_code = self.process.wait(timeout=force_timeout_s)
            except subprocess.TimeoutExpired:
               self.process.terminate()
               try:
                  self.exit_code = self.process.wait(timeout=2.0)
               except subprocess.TimeoutExpired:
                  self.process.kill()
                  self.exit_code = self.process.wait(timeout=2.0)
      else:
         self.exit_code = self.process.returncode

      if self.process.stdout is not None:
         print("=== server stdout/stderr ===")
         print(self.process.stdout.read())
         print("=== end server stdout/stderr ===")

      print(f"Server process exit code: {self.exit_code}")
      self.process = None
      return int(self.exit_code)

   def stop_via_system_signal(self, graceful_timeout_s: float = 5.0, force_timeout_s: float = 3.0) -> int:
      if self.process is None:
         return self.exit_code if self.exit_code is not None else 0

      if self.process.poll() is None:
         if os.name == "nt":
            self.process.send_signal(signal.CTRL_BREAK_EVENT)
         else:
            os.killpg(self.process.pid, signal.SIGINT)

         try:
            self.exit_code = self.process.wait(timeout=graceful_timeout_s)
         except subprocess.TimeoutExpired:
            self.process.terminate()
            try:
               self.exit_code = self.process.wait(timeout=force_timeout_s)
            except subprocess.TimeoutExpired:
               self.process.kill()
               self.exit_code = self.process.wait(timeout=2.0)
      else:
         self.exit_code = self.process.returncode

      if self.process.stdout is not None:
         print("=== server stdout/stderr ===")
         print(self.process.stdout.read())
         print("=== end server stdout/stderr ===")

      print(f"Server process exit code (signal shutdown): {self.exit_code}")
      self.process = None
      return int(self.exit_code)

@pytest.fixture(autouse=True)
def server_process_fixture(request: pytest.FixtureRequest):
   request_node = cast(Any, request).node
   if request_node.get_closest_marker("no_server_harness") is not None:
      yield
      return

   harness = CryptoAgentHarness()
   harness.start()
   yield harness
   harness.stop()
   # this would confuse Jenkins
   #request.node.user_properties.append(("server_exit_code", str(exit_code)))

def _RunAgentWithConfig(config_file_path: str, timeout_s: float = 5.0) -> subprocess.CompletedProcess[str]:
   return _RunAgentWithArgs(["-c", str(config_file_path)], timeout_s)


def _RunAgentWithArgs(args: Sequence[str], timeout_s: float = 5.0) -> subprocess.CompletedProcess[str]:
   binary_path = CryptoAgentHarness.ResolveBinaryPath()
   process_args = CryptoAgentHarness.BuildProcessArgs(binary_path, args)

   return subprocess.run(
      process_args,
      cwd=str(binary_path.parent),
      stdout=subprocess.PIPE,
      stderr=subprocess.STDOUT,
      text=True,
      timeout=timeout_s,
   )


def _GetExpectedLogFilePath(config_file_path: str) -> pathlib.Path:
   binary_path = CryptoAgentHarness.ResolveBinaryPath()
   config_abs_path = (binary_path.parent / config_file_path).resolve()
   log_folder = "./logs"

   with config_abs_path.open("r", encoding="utf-8") as config_file:
      for line in config_file:
         stripped_line = line.strip()
         if stripped_line.startswith("OSY_CRYPTO_AGENT_LOG_FILE_FOLDER="):
            log_folder = stripped_line.split("=", 1)[1].strip()
            break

   return (binary_path.parent / log_folder / "osy_crypto_agent.log").resolve()


class TestsCryptoAgent:
   @pytest.mark.no_server_harness
   @pytest.mark.parametrize("help_option", ["-h", "--help"])
   def test_help_option_exits_without_starting_server(self, help_option: str):
      result = _RunAgentWithArgs([help_option], timeout_s=2.0)
      assert result.returncode == 10

      with pytest.raises(Exception):
         DoTcpRequest(_BuildServiceRequest(0x00), timeout_s=0.5)

   def test_ping(self):
      response_data = DoTcpRequest(_BuildServiceRequest(0x00))
      #minimum: 4 size header; 1 service ID; 1 result; 2 protocol version, 2 reserved, 2 size name, 2 size version
      _AssertResponseHeader(response_data, 0x00, 0x00, minimum_payload_size=8)
      assert (response_data[6] == 0x00)
      assert (response_data[7] == 0x01)
      assert (response_data[8] == 0x00)
      assert (response_data[9] == 0x00)

   def test_unknown_service(self):
      response_data = DoTcpRequest(_BuildServiceRequest(0x03))
      #at least: 4 size header; 1 service ID; 1 negative result; 2 size of string
      _AssertResponseHeader(response_data, 0x03, 0x01, minimum_payload_size=2)

   def test_solve_negative(self):
      response_data = DoTcpRequest(_CreateSolveRequest(
         b"\x11\x22\x33\x44\x55\x66\x11\x22\x33\x44\x55\x66\x11\x22\x33\x44\x55\x66\x11\x22"))
      #at least: 4 size header; 1 service ID; 1 negative result; 2 size of string
      _AssertResponseHeader(response_data, 0x02, 0x03, minimum_payload_size=2)

   def test_solve_positive(self):
      response_data = DoTcpRequest(_CreateSolveRequest(
         # see valid_1_private_key_crt.pem
         b"\x40\x00\xa5\xc3\x6b\x82\xa0\x16\x81\xdb\x77\x96\xfb\xa4\xcf\x3b\xdd\x77\x18\x24"))
      assert (len(response_data) == (4 + 1 + 1 + 2 + 128))
      _AssertResponseHeader(response_data, 0x02, 0x00, minimum_payload_size=2 + 128)
      assert (response_data[6] == 0x00)
      assert (response_data[7] == 128)

      expected_signature = bytes.fromhex(
         "49e25200e5e73e6921945d1e22ff1a7623a467f34384ff2c5bdb8e29aa5ea0be"
         "0b2e2308c6f2d5320dcf9547e67b7300cc2212caa24eef866df49ad15e31167c"
         "2985e0484e49e910ec691cc80ffbf437d55773ed7db57f89486582a960340a2d"
         "a810a59cf15aaf3881fcd15967f00108e96e7abbb7738f602d37b9892382efcd"
      )
      assert (response_data[8:8+128] == expected_signature)

   def test_pem_folder_rescan_triggered_on_file_added_removed(self):
      # SNR for valid_1_private_key_crt.pem
      serial_number_valid_1 = (
         b"\x40\x00\xa5\xc3\x6b\x82\xa0\x16\x81\xdb\x77\x96\xfb\xa4\xcf\x3b\xdd\x77\x18\x24"
      )

      cert_folder = pathlib.Path(__file__).resolve().parent / "certificates"
      pem_file = cert_folder / "valid_1_private_key_crt.pem"
      pem_file_renamed = cert_folder / "valid_1_private_key_crt.pem.bak"

      # Confirm baseline: solving with valid_1 certificate succeeds
      response = DoTcpRequest(_CreateSolveRequest(serial_number_valid_1))
      _AssertResponseHeader(response, 0x02, 0x00, minimum_payload_size=2 + 128)

      try:
         # Rename .pem → .pem.bak so TglFileFind no longer includes it in the folder scan.
         # The next solve_auth_challenge call triggers m_PemFolderReloadNeeded() which detects
         # the changed folder hash and reloads — the certificate is now absent.
         pem_file.rename(pem_file_renamed)

         response = DoTcpRequest(_CreateSolveRequest(serial_number_valid_1))
         _AssertResponseHeader(response, 0x02, 0x03, minimum_payload_size=2)

      finally:
         # Always restore the file so other tests are not affected
         if pem_file_renamed.exists():
            pem_file_renamed.rename(pem_file)

      # Rename back → folder hash changes again → rescan → certificate reloaded → solve succeeds
      response = DoTcpRequest(_CreateSolveRequest(serial_number_valid_1))
      _AssertResponseHeader(response, 0x02, 0x00, minimum_payload_size=2 + 128)

   # do not autorun server binary; we call it here manually
   @pytest.mark.no_server_harness
   def test_parse_parameters_fails_on_missing_config_file(self):
      result = _RunAgentWithConfig("../src/test/does_not_exist.conf")
      assert result.returncode == 10

   @pytest.mark.no_server_harness
   def test_init_fails_on_missing_pem_folder(self):
      result = _RunAgentWithConfig("../src/test/osy_crypto_agent_non_existing_pem_folder.conf");
      assert result.returncode == 20

   @pytest.mark.no_server_harness
   def test_creates_log_file_in_configured_folder(self):
      config_path = "../src/test/osy_crypto_agent_positive.conf"
      log_file_path = _GetExpectedLogFilePath(config_path)
      log_file_path.parent.mkdir(parents=True, exist_ok=True)

      # remove file if it already exists
      if log_file_path.exists():
         log_file_path.unlink()

      harness = CryptoAgentHarness()
      harness.start(args=["-c", config_path])
      harness.stop()

      assert log_file_path.is_file()

   @pytest.mark.no_server_harness
   def test_close_via_system_signal(self):
      harness = CryptoAgentHarness()
      harness.start()

      exit_code = harness.stop_via_system_signal()
      assert exit_code == 0

      with pytest.raises(Exception):
         DoTcpRequest(_BuildServiceRequest(0x00), timeout_s=0.5)

   @pytest.mark.no_server_harness
   def test_close_via_tcp_service(self):
      harness = CryptoAgentHarness()
      harness.start()

      shutdown_response = DoTcpRequest(_BuildServiceRequest(0x01), timeout_s=1.0)
      _AssertResponseHeader(shutdown_response, 0x01, 0x00)

      exit_code = harness.stop(graceful_timeout_s=5.0, force_timeout_s=1.0)
      assert exit_code == 0

      with pytest.raises(Exception):
         DoTcpRequest(_BuildServiceRequest(0x00), timeout_s=0.5)

   @pytest.mark.no_server_harness
   def test_server_bind_all_interfaces(self):
      config_path = "../src/test/osy_crypto_agent_bind_all_interfaces.conf"
      harness = CryptoAgentHarness()
      harness.start(args=["-c", config_path])

      ping_response = DoTcpRequest(_BuildServiceRequest(0x00), timeout_s=1.0)
      _AssertResponseHeader(ping_response, 0x00, 0x00)

      exit_code = harness.stop(graceful_timeout_s=5.0, force_timeout_s=1.0)
      assert exit_code == 0

   @pytest.mark.no_server_harness
   def test_server_uses_alternative_port_from_config(self):
      config_path = "../src/test/osy_crypto_agent_alternative_port.conf"
      harness = CryptoAgentHarness(server_port=ALTERNATIVE_SERVER_PORT)
      harness.start(args=["-c", config_path])

      ping_response = DoTcpRequest(_BuildServiceRequest(0x00), timeout_s=1.0,
                                   server_port=ALTERNATIVE_SERVER_PORT)
      _AssertResponseHeader(ping_response, 0x00, 0x00)

      with pytest.raises(Exception):
         DoTcpRequest(_BuildServiceRequest(0x00), timeout_s=0.5)

      exit_code = harness.stop(graceful_timeout_s=5.0, force_timeout_s=1.0)
      assert exit_code == 0

   @pytest.mark.no_server_harness
   def test_init_fails_on_invalid_bind_address(self):
      result = _RunAgentWithConfig("../src/test/osy_crypto_agent_bind_invalid_address.conf")
      assert result.returncode == 30

   @pytest.mark.no_server_harness
   def test_server_rejects_connection_from_different_bind_address(self):
      config_path = "../src/test/osy_crypto_agent_bind_localhost_alternate.conf"
      harness = CryptoAgentHarness()

      # Start server without ping verification (since it binds to 127.0.0.2, not 127.0.0.1)
      harness.start_raw(args=["-c", config_path])

      # Give server time to start and bind
      time.sleep(0.5)

      # Attempt to connect from 127.0.0.1 should fail (server binds to 127.0.0.2)
      with pytest.raises(Exception):
         DoTcpRequest(_BuildServiceRequest(0x00), timeout_s=1.0)

      exit_code = harness.stop(graceful_timeout_s=5.0, force_timeout_s=1.0)
      assert exit_code == 0