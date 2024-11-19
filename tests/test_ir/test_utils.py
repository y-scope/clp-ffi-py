import json
import random
import time
import unittest
from datetime import tzinfo
from math import floor
from pathlib import Path
from typing import Any, Generator, IO, List, Optional, Set, Tuple, Union

import dateutil.tz
from smart_open import register_compressor  # type: ignore
from zstandard import (
    ZstdCompressionWriter,
    ZstdCompressor,
    ZstdDecompressionReader,
    ZstdDecompressor,
)

from clp_ffi_py.ir import (
    LogEvent,
    Metadata,
    Query,
)
from clp_ffi_py.wildcard_query import WildcardQuery


class JsonLinesFileReader:
    """
    Class for reading JSON files.

    It assumes each line in the file is a JSON string, and the parser parses each line into a JSON
    object, and returns them through a generator.
    """

    def __init__(self, file_path: Path):
        """
        Initializes the `JSONFileReader` with the given file path.

        :param file_path: Path to the JSON file to read.
        """
        self.file_path: Path = file_path

    def read_lines(self) -> Generator[Any, None, None]:
        """
        Reads each line in the JSON file, parses it as a JSON object, and yields the JSON object.

        :yield: Parsed JSON object for each line in the file.
        """
        with open(self.file_path, "r", encoding="utf-8") as file:
            for line in file:
                yield json.loads(line.strip())


def _zstd_compressions_handler(
    file_obj: IO[bytes], mode: str
) -> Union[ZstdCompressionWriter, ZstdDecompressionReader]:
    if "wb" == mode:
        cctx = ZstdCompressor()
        return cctx.stream_writer(file_obj)
    elif "rb" == mode:
        dctx = ZstdDecompressor()
        return dctx.stream_reader(file_obj)
    else:
        raise RuntimeError(f"Zstd handler: Unexpected Mode {mode}")


# Register .zst with zstandard library compressor
register_compressor(".zst", _zstd_compressions_handler)


def get_current_timestamp() -> int:
    """
    :return: the current Unix epoch time in milliseconds.
    """
    timestamp_ms: int = floor(time.time() * 1000)
    return timestamp_ms


class TestCLPBase(unittest.TestCase):
    """
    Base class for all the testers.

    Helper functions should be defined here.
    """

    def _check_metadata(
        self,
        metadata: Metadata,
        expected_ref_timestamp: int,
        expected_timestamp_format: str,
        expected_timezone_id: str,
        extra_test_info: str = "",
    ) -> None:
        """
        Given a Metadata object, check if the content matches the reference.

        :param metadata: Metadata object to be checked.
        :param expected_ref_timestamp: Expected reference timestamp.
        :param expected_timestamp_format: Expected timestamp format.
        :param expected_timezone_id: Expected timezone ID.
        :param extra_test_info: Extra test information appended to the assert message.
        """
        ref_timestamp: int = metadata.get_ref_timestamp()
        timestamp_format: str = metadata.get_timestamp_format()
        timezone_id: str = metadata.get_timezone_id()
        timezone: tzinfo = metadata.get_timezone()

        self.assertEqual(
            ref_timestamp,
            expected_ref_timestamp,
            f'Reference Timestamp: "{ref_timestamp}", Expected: "{expected_ref_timestamp}"\n'
            + extra_test_info,
        )
        self.assertEqual(
            timestamp_format,
            expected_timestamp_format,
            f'Timestamp Format: "{timestamp_format}", Expected: "{expected_timestamp_format}"\n'
            + extra_test_info,
        )
        self.assertEqual(
            timezone_id,
            expected_timezone_id,
            f'Timezone ID: "{timezone_id}", Expected: "{expected_timezone_id}"\n',
        )

        expected_tzinfo: Optional[tzinfo] = dateutil.tz.gettz(expected_timezone_id)
        assert expected_tzinfo is not None
        is_the_same_tz: bool = expected_tzinfo is timezone
        self.assertEqual(
            is_the_same_tz,
            True,
            f"Timezone does not match timezone id. Timezone ID: {timezone_id}, Timezone:"
            f' {str(timezone)}"\n' + extra_test_info,
        )

    def _check_log_event(
        self,
        log_event: LogEvent,
        expected_log_message: str,
        expected_timestamp: int,
        expected_idx: int,
        extra_test_info: str = "",
    ) -> None:
        """
        Given a LogEvent object, check if the content matches the reference.

        :param log_event: LogEvent object to be checked.
        :param expected_log_message: Expected log message.
        :param expected_timestamp: Expected timestamp.
        :param expected_idx: Expected log event index.
        :param extra_test_info: Extra test information appended to the assert message.
        """
        log_message: str = log_event.get_log_message()
        timestamp: int = log_event.get_timestamp()
        idx: int = log_event.get_index()
        self.assertEqual(
            log_message,
            expected_log_message,
            f'Log message: "{log_message}", Expected: "{expected_log_message}"\n' + extra_test_info,
        )
        self.assertEqual(
            timestamp,
            expected_timestamp,
            f'Timestamp: "{timestamp}", Expected: "{expected_log_message}"\n' + extra_test_info,
        )
        self.assertEqual(
            idx, expected_idx, f"Message idx: {idx}, Expected: {expected_idx}\n" + extra_test_info
        )

    def _check_wildcard_query(
        self, wildcard_query: WildcardQuery, ref_wildcard_string: str, ref_is_case_sensitive: bool
    ) -> None:
        """
        Given a WildcardQuery object, check if the stored data matches the input reference.

        :param wildcard_query: Input WildcardQuery object.
        :param ref_wildcard_string: Reference wildcard string.
        :param ref_is_case_sensitive: Reference case-sensitive indicator.
        """
        wildcard_string: str = wildcard_query.wildcard_query
        is_case_sensitive: bool = wildcard_query.case_sensitive
        self.assertEqual(
            wildcard_string,
            ref_wildcard_string,
            f'Wildcard string: "{wildcard_string}"; Expected: "{ref_wildcard_string}"',
        )
        self.assertEqual(
            is_case_sensitive,
            ref_is_case_sensitive,
            f"Expected case-sensitive indicator: {ref_is_case_sensitive}",
        )

    def _check_query(
        self,
        query: Query,
        ref_search_time_lower_bound: int,
        ref_search_time_upper_bound: int,
        ref_wildcard_queries: Optional[List[WildcardQuery]],
        ref_search_time_termination_margin: int,
    ) -> None:
        """
        Given a Query object, check if the stored data matches the input references.

        :param query: Input Query object to validate.
        :param ref_search_time_lower_bound: Reference search time lower bound.
        :param ref_search_time_upper_bound: Reference search time upper bound.
        :param ref_wildcard_queries: Reference wildcard query list.
        :param ref_search_time_termination_margin: Reference search time termination margin.
        """
        search_time_lower_bound: int = query.get_search_time_lower_bound()
        search_time_upper_bound: int = query.get_search_time_upper_bound()
        wildcard_queries: Optional[List[WildcardQuery]] = query.get_wildcard_queries()
        search_time_termination_margin: int = query.get_search_time_termination_margin()
        self.assertEqual(
            search_time_lower_bound,
            ref_search_time_lower_bound,
            f"Search time lower bound: {search_time_lower_bound}; Expected:"
            f" {ref_search_time_lower_bound}",
        )
        self.assertEqual(
            search_time_upper_bound,
            ref_search_time_upper_bound,
            f"Search time upper bound: {search_time_upper_bound}; Expected:"
            f" {ref_search_time_upper_bound}",
        )
        self.assertEqual(
            search_time_termination_margin,
            ref_search_time_termination_margin,
            f"Search time termination margin: {search_time_termination_margin}; Expected:"
            f" {ref_search_time_termination_margin}",
        )

        if None is ref_wildcard_queries:
            self.assertEqual(wildcard_queries, None, "The wildcard query list should be empty.")
            return

        assert None is not wildcard_queries
        wildcard_query_list_size: int = len(wildcard_queries)
        self.assertEqual(
            wildcard_query_list_size,
            len(wildcard_queries),
            "Wildcard query list size doesn't match.",
        )

        for i in range(wildcard_query_list_size):
            self._check_wildcard_query(
                wildcard_queries[i],
                ref_wildcard_queries[i].wildcard_query,
                ref_wildcard_queries[i].case_sensitive,
            )


class LogGenerator:
    """
    Generates random logs or wildcard queries from a list of log types and dictionary words.
    """

    log_type_list: List[str] = [
        (
            "org.apache.hadoop.yarn.event.AsyncDispatcher: Registering class"
            " org.apache.hadoop.yarn.server.nodemanager.NodeManagerEventType for class"
            " org.apache.hadoop.yarn.server.nodemanager.NodeManager"
        ),
        "\d: Scheduled snapshot period at \i second(s).",
        (
            "org.apache.hadoop.ipc.Client: Retrying connect to server: \d:\d Already tried \i"
            " time(s); retry policy is RetryUpToMaximumCountWithFixedSleep(maxRetries=\i,"
            " sleepTime=\i MILLISECONDS)"
        ),
        (
            "org.apache.hadoop.yarn.server.nodemanager.containermanager.monitor.ContainersMonitor:"
            " Memory usage of ProcessTree \i for container-id \d: \f MB of \i GB physical memory"
            " used; \f MB of \f GB virtual memory used"
        ),
        (
            "org.apache.hadoop.hdfs.server.datanode.DataNode: Namenode Block pool \d (Datanode Uuid"
            " \d) service to \d:\i trying to claim ACTIVE state with txid=\i"
        ),
        " org.apache.hadoop.mapred.MapTask: (RESET) equator \i kv \i(\i) kvi \i(\i)",
        " org.apache.hadoop.mapred.TaskAttemptListenerImpl: Progress of TaskAttempt \d is : \f",
        (
            " \d: Final Stats: PendingReds:\i ScheduledMaps:\i ScheduledReds:\i AssignedMaps:\i"
            " AssignedReds:\i CompletedMaps:\i CompletedReds:\i ContAlloc:\i ContRel:\i"
            " HostLocal:\i RackLocal:\i"
        ),
        (
            "org.apache.hadoop.yarn.server.resourcemanager.scheduler.capacity.LeafQueue: Reserved"
            " container  application=\d resource=<memory:\i, vCores:\i> queue=\d: capacity=\f,"
            " absoluteCapacity=\f, usedResources=<memory:\i, vCores:\i>, usedCapacity=\f,"
            " absoluteUsedCapacity=\f, numApps=\i, numContainers=\i usedCapacity=\f"
            " absoluteUsedCapacity=\f used=<memory:\i, vCores:\i> cluster=<memory:\i, vCores:\i>"
        ),
        "org.apache.hadoop.hdfs.server.namenode.TransferFsImage: Transfer took \d at \f KB/s",
    ]

    dict_words: List[str] = [
        "attempt_1427088391284_0029_r_000026_0",
        "blk_1073742594_1770",
        "container_1427088391284_0034_01_000074",
        "DFSClient_attempt_1427088391284_0015_r_000003_0_-1867302407_1",
        "ip-172-31-17-96",
        "jvm_1427088391284_0034_m_000018",
        "jvm_1427088391284_0032_m_000200",
        "task_1427088391284_0034_m_000005",
        (
            "/tmp/hadoop-ubuntu/nm-local-dir/usercache/ubuntu/appcache/application_1427088391284_72"
            "/container_1427088391284_0072_01_000031/default_container_executor.sh"
        ),
        (
            "/tmp/hadoop-ubuntu/nm-local-dir/usercache/ubuntu/appcache/application_1427088391284_27"
            "/container_1427088391284_0027_01_000008.tokens"
        ),
        (
            "/tmp/hadoop-ubuntu/nm-local-dir/usercache/ubuntu/appcache/application_1427088391284_87"
            "/container_1427088391284_0087_01_000043/default_container_executor.sh"
        ),
        "/tmp/hadoop-yarn/staging/ubuntu/.staging/job_1427088391284_0005",
        (
            "/tmp/hadoop-ubuntu/dfs/data/current/BP-1121897155-172.31.17.135-1427088167814/current"
            "/finalized/subdir0/subdir18/blk_1073746642"
        ),
        "328418859ns",
        "3154ms",
    ]

    @staticmethod
    def generate_random_logs(num_log_events: int) -> Tuple[Metadata, List[LogEvent]]:
        """
        Generates logs randomly by using log types specified in `log_type_list`. Each log type
        contains placeholders, and each placeholder will be randomly replaced by randomly generated
        integers, randomly generated floating point numbers, and randomly selected dictionary words
        according to the type.

        :param num_log_events: Number of log events to generate.
        :return: A tuple containing the generated log events and the metadata.
        """
        ref_timestamp: int = get_current_timestamp()
        timestamp_format: str = "yy/MM/dd HH:mm:ss"
        timezone_id: str = "America/Chicago"
        metadata: Metadata = Metadata(ref_timestamp, timestamp_format, timezone_id)
        log_events: List[LogEvent] = []
        timestamp: int = ref_timestamp
        for idx in range(num_log_events):
            log_message: str = random.choice(LogGenerator.log_type_list)
            log_message = log_message.replace("\d", random.choice(LogGenerator.dict_words))
            log_message = log_message.replace("\i", str(random.randint(-999999999, 999999999)))
            log_message = log_message.replace("\f", str(random.uniform(-999999, 9999999)))
            timestamp += random.randint(0, 10)
            log_event: LogEvent = LogEvent(log_message + "\n", timestamp, idx)
            log_events.append(log_event)
        return metadata, log_events

    @staticmethod
    def generate_random_log_type_wildcard_queries(num_wildcard_queries: int) -> List[WildcardQuery]:
        """
        Generates wildcard queries randomly from log types. A randomly selected
        log type will be translated into a wildcard query by:
        1. replacing all the placeholders by `*`
        2. replacing a random character by `?`

        :param num_wildcard_queries: Number of wildcard queries to generate.
        Each wildcard query will correspond to a unique log type. If the given
        number is larger than the number of available log types, it will
        generate wildcard queries only up to the number of existing log types.
        :return: A list of generated wildcard queries, each is presented as an
        instance of WildcardQuery.
        """
        num_log_types: int = len(LogGenerator.log_type_list)
        num_wildcard_queries = min(num_log_types, num_wildcard_queries)
        selected_log_type_idx: Set[int] = set()
        max_log_type_idx: int = num_log_types - 1
        wildcard_queries: List[WildcardQuery] = []
        for _ in range(num_wildcard_queries):
            idx: int
            while True:
                idx = random.randint(0, max_log_type_idx)
                if idx in selected_log_type_idx:
                    continue
                break
            selected_log_type_idx.add(idx)
            wildcard_query_str: str = LogGenerator.log_type_list[idx]

            # Replace the placeholders by the wildcard `*`
            wildcard_query_str = wildcard_query_str.replace("\d", "*")
            wildcard_query_str = wildcard_query_str.replace("\i", "*")
            wildcard_query_str = wildcard_query_str.replace("\f", "*")

            # Replace a random character that is not `*` by `?`
            str_idx_max: int = len(wildcard_query_str) - 1
            while True:
                str_idx: int = random.randint(0, str_idx_max)
                if "*" == wildcard_query_str[str_idx] or "/" == wildcard_query_str[str_idx]:
                    continue
                wildcard_query_str = (
                    wildcard_query_str[:str_idx] + "?" + wildcard_query_str[str_idx + 1 :]
                )
                break

            wildcard_queries.append(
                WildcardQuery(wildcard_query=wildcard_query_str, case_sensitive=True)
            )

        return wildcard_queries
