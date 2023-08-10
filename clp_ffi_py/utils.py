from datetime import datetime, tzinfo
from typing import Optional

import dateutil.tz


def get_formatted_timestamp(timestamp: int, timezone: Optional[tzinfo]) -> str:
    """
    Gets the formatted timestamp string from the provided timestamp with the
    provided timezone using isoformat.

    :param timestamp: Timestamp to format.
    :param timezone: Timezone of timestamp parameter. If None is given, UTC is
        used by default.
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
