"""
This file contains clp IR four-byte encoding methods.
"""

def encode_preamble(ref_timestamp: int, timestamp_format: str, timezone: str) -> bytearray:
    """
    Create the encoded CLP preamble for a stream of encoded log messages.

    :param ref_timestamp: Reference timestamp used to calculate deltas emitted
    with each message.
    :param timestamp_format: Timestamp format to be use when generating the
    logs with a reader.
    :param timezone: Timezone in TZID format to be use when generating the
    timestamp from Unix epoch time.
    :raises NotImplementedError: If metadata length too large.
    :return: The encoded preamble.
    """
    ...

def encode_message_and_timestamp_delta(timestamp_delta: int, msg: bytes) -> bytearray:
    """
    Encode the log `msg` along with the timestamp delta.

    :param timestamp_delta: Timestamp difference in miliseconds between the
    current log message and the previous log message.
    :param msg: Log message to encode.
    :raises NotImplementedError: If the log message failed to encode, or the
    timestamp delta exceeds the supported size.
    :return: The encoded message and timestamp.
    """
    ...

def encode_message(msg: bytes) -> bytearray:
    """
    Encode the log `msg`.

    :param msg: Log message to encode.
    :raises NotImplementedError: If the log message failed to encode.
    :return: The encoded message.
    """
    ...

def encode_timestamp_delta(timestamp_delta: int) -> bytearray:
    """
    Encode the timestamp.

    :param timestamp_delta: Timestamp difference in miliseconds between the
    current log message and the previous log message.
    :raises NotImplementedError: If the timestamp delta exceeds the supported
    size.
    :return: The encoded timestamp.
    """
    ...
