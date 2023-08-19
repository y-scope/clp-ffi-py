import os
import sys

current_directory: str = os.path.dirname(os.path.abspath(__file__))
if current_directory.endswith("tests") and current_directory not in sys.path:
    sys.path.append(current_directory)
