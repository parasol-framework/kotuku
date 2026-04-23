//********************************************************************************************************************
// XML Entity and Character Reference Decoding Interface
//
// Provides functions for decoding XML entity references and numeric character references.  These utilities
// handle the conversion of XML escape sequences back to their literal character representations.
//
// Key functions:
//   - decode_numeric_reference: Decodes &#xHH; and &#DDD; character references to UTF-8
//   - unescape_all: Processes all entity references in an XML document's tag content
//
// This module ensures proper handling of XML escape sequences whilst maintaining Unicode correctness.

#pragma once

#include <cstddef>
#include "xml.h"

bool decode_numeric_reference(const char *Start, size_t Length, char *Buffer, size_t &ResultLength);
void unescape_all(extXML *Self, TAGS &Tags);
