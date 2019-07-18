#pragma once

#include <Core/Types.h>

namespace DB
{
class Context;

/// Extracts information about where the format schema file is from passed context and keep it.
class FormatSchemaInfo
{
public:
    FormatSchemaInfo(const Context & context, const String & format);

    /// Returns path to the schema file.
    const String & schemaPath() const { return schema_path; }
    String absoluteSchemaPath() const { return schema_directory + schema_path; }

    /// Returns directory containing the schema file.
    const String & schemaDirectory() const { return schema_directory; }

    /// Returns name of the message type.
    const String & messageName() const { return message_name; }

    /// Returns maximum expected message length
    UInt64 maxMessageSize() const { return max_message_size; }

private:
    String schema_path;
    String schema_directory;
    String message_name;
    UInt64 max_message_size;
};

}
