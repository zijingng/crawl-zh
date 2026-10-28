"""Hash text dependencies consistently across Windows and Unix checkouts."""
import hashlib


def text_file_sha256(path):
    # Only normalize the platform line ending. All other bytes (including
    # spaces, encoding, and standalone CR) remain part of the reviewed source.
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()
