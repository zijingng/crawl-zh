"""Compatibility helpers for localization builds on Python 3.6 and later."""


def pgettext(translations, context, message):
    """Look up a contextual message without requiring Python 3.8."""
    key = context + '\x04' + message
    translated = translations.gettext(key)
    return message if translated == key else translated


def is_relative_to(path, parent):
    """Check containment without requiring pathlib's Python 3.9 method."""
    try:
        path.relative_to(parent)
    except ValueError:
        return False
    return True
