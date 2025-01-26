import json
from datetime import datetime, tzinfo
from typing import Any, Dict, Optional

import dateutil.tz
import msgpack


def get_formatted_timestamp(timestamp: int, timezone: Optional[tzinfo]) -> str:
    """
    Gets the formatted timestamp string from the provided timestamp with the provided timezone using
    isoformat.

    :param timestamp: Timestamp to format.
    :param timezone: Timezone of timestamp parameter. If None is given, UTC is used by default.
    :return: String of formatted timestamp.
    """
    if timezone is None:
        timezone = dateutil.tz.UTC
    dt: datetime = datetime.fromtimestamp(timestamp / 1000, timezone)
    return dt.isoformat(sep=" ", timespec="milliseconds")


def get_timezone_from_timezone_id(timezone_id: str) -> tzinfo:
    """
    Gets the Python timezone object of the provided timezone id.

    :param timezone_id: Timezone Id.
    :return: Timezone object.
    :raises: RuntimeError The given timestamp ID is invalid.
    """
    timezone: Optional[tzinfo] = dateutil.tz.gettz(timezone_id)
    if timezone is None:
        raise RuntimeError(f"Invalid timezone id: {timezone_id}")
    return timezone


def serialize_dict_to_msgpack(dictionary: Dict[Any, Any]) -> bytes:
    """
    Serializes the given dictionary into msgpack.

    :param dictionary: The dictionary to serialize.
    :return: msgpack byte sequence.
    :raises: TypeError The given input is not a dictionary.
    """
    if not isinstance(dictionary, dict):
        raise TypeError("The type of the input object must be a dictionary.")
    return msgpack.packb(dictionary)


def serialize_dict_to_json_str(dictionary: Dict[Any, Any]) -> str:
    """
    Serializes the given dictionary into a JSON string.

    :param dictionary: The dictionary to serialize.
    :return: JSON string of the serialized dictionary.
    :raises: TypeError The given input is not a dictionary.
    """
    if not isinstance(dictionary, dict):
        raise TypeError("The type of the input object must be a dictionary.")
    return json.dumps(dictionary)


def parse_json_str(json_str: str) -> Any:
    """
    Wrapper of `json.loads`, which parses a JSON string into a Python object.

    :param json_str: The JSON string to parse.
    :return: The parsed JSON object.
    """
    return json.loads(json_str)
