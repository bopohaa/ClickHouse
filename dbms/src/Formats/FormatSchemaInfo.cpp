#include <Formats/FormatSchemaInfo.h>
#include <Poco/Path.h>
#include <Interpreters/Context.h>
#include <Common/Exception.h>
#include <IO/ReadHelpers.h>


namespace DB
{
namespace ErrorCodes
{
    extern const int BAD_ARGUMENTS;
}


namespace
{
    String getFormatSchemaDefaultFileExtension(const String & format)
    {
        if (format == "Protobuf")
            return "proto";
        else if (format == "CapnProto")
            return "capnp";
        else
            return "";
    }
}


FormatSchemaInfo::FormatSchemaInfo(const Context & context, const String & format)
{
    String format_schema = context.getSettingsRef().format_schema.toString();
    if (format_schema.empty())
        throw Exception(
            "The format " + format + " requires a schema. The 'format_schema' setting should be set", ErrorCodes::BAD_ARGUMENTS);

    String default_file_extension = getFormatSchemaDefaultFileExtension(format);

    size_t colon_pos = format_schema.find(':');
    Poco::Path path;
    if ((colon_pos == String::npos) || (colon_pos == 0) || (colon_pos == format_schema.length() - 1) || (colon_pos + 1 == '#')
        || path.assign(format_schema.substr(0, colon_pos)).makeFile().getFileName().empty())
    {
        throw Exception(
            "Format schema requires the 'format_schema' setting to have the 'schema_file:message_name' format"
                + (default_file_extension.empty() ? "" : ", e.g. 'schema." + default_file_extension + ":Message'") + ". Got '" + format_schema
                + "'",
            ErrorCodes::BAD_ARGUMENTS);
    }

    max_message_size = 0;
    size_t size_pos = format_schema.find('#', colon_pos);
    ++colon_pos;
    if (size_pos != std::string::npos) {
        if (size_pos != format_schema.length() - 1)
            max_message_size = parse<UInt64>(format_schema.substr(size_pos + 1));
    
        size_pos -= colon_pos;
    }

    message_name = format_schema.substr(colon_pos, size_pos);

    auto default_schema_directory = [&context]()
    {
        static const String str = Poco::Path(context.getFormatSchemaPath()).makeAbsolute().makeDirectory().toString();
        return str;
    };
    auto is_server = [&context]()
    {
        return context.hasGlobalContext() && (context.getGlobalContext().getApplicationType() == Context::ApplicationType::SERVER);
    };

    if (path.getExtension().empty() && !default_file_extension.empty())
        path.setExtension(default_file_extension);

    if (path.isAbsolute())
    {
        if (is_server())
            throw Exception("Absolute path in the 'format_schema' setting is prohibited: " + path.toString(), ErrorCodes::BAD_ARGUMENTS);
        schema_path = path.getFileName();
        schema_directory = path.makeParent().toString();
    }
    else if (path.depth() >= 1 && path.directory(0) == "..")
    {
        if (is_server())
            throw Exception(
                "Path in the 'format_schema' setting shouldn't go outside the 'format_schema_path' directory: " + path.toString(),
                ErrorCodes::BAD_ARGUMENTS);
        path = Poco::Path(default_schema_directory()).resolve(path).toString();
        schema_path = path.getFileName();
        schema_directory = path.makeParent().toString();
    }
    else
    {
        schema_path = path.toString();
        schema_directory = default_schema_directory();
    }
}

}
