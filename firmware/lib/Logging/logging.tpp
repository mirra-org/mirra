constexpr std::string_view Log::levelToString(Level level)
{
    switch (level)
    {
    case Level::ERROR:
        return "ERROR: ";
    case Level::INFO:
        return "INFO: ";
    case Level::DEBUG:
        return "DEBUG: ";
    }
    return "NONE: ";
}

template <Log::Level level> size_t Log::printPreamble(const tm& time)
{
    constexpr size_t timeLength{sizeof("[0000-00-00 00:00:00]")};
    strftime(buffer, timeLength, "[%F %T]", &time);
    constexpr std::string_view levelString{levelToString(level)};
    strcpy(&buffer[timeLength - 1], levelString.cbegin());
    return timeLength + levelString.size() - 1;
}

template <class T> constexpr std::string_view rawTypeToFormatSpecifier();
template <> constexpr std::string_view rawTypeToFormatSpecifier<const char*>() { return "%s"; }
template <> constexpr std::string_view rawTypeToFormatSpecifier<char*>() { return "%s"; }
template <> constexpr std::string_view rawTypeToFormatSpecifier<signed int>() { return "%i"; }
template <> constexpr std::string_view rawTypeToFormatSpecifier<unsigned int>() { return "%u"; }
template <> constexpr std::string_view rawTypeToFormatSpecifier<long signed int>() { return "%i"; }
template <> constexpr std::string_view rawTypeToFormatSpecifier<signed char>() { return "%i"; }
template <> constexpr std::string_view rawTypeToFormatSpecifier<unsigned char>() { return "%u"; }
template <> constexpr std::string_view rawTypeToFormatSpecifier<float>() { return "%f"; }
template <> constexpr std::string_view rawTypeToFormatSpecifier<char>() { return "%c"; }

// TODO: Clean up the type to format specifier compile-time functionality with proper SFINAE. This
// here works, but it's messy, requires two functions, and is unruly to extend.

template <class T> constexpr std::string_view typeToFormatSpecifier()
{
    using rawType = std::decay_t<T>;
    if constexpr (std::is_enum_v<rawType>)
    {
        using underlyingType = std::underlying_type_t<rawType>;
        if constexpr (std::is_same_v<std::make_unsigned_t<underlyingType>, unsigned char>)
        {
            if constexpr (std::is_signed_v<underlyingType>)
            {
                return rawTypeToFormatSpecifier<signed int>();
            }
            else
            {
                return rawTypeToFormatSpecifier<unsigned int>();
            }
        }
        else
        {
            return rawTypeToFormatSpecifier<underlyingType>();
        }
    }
    else
    {
        return rawTypeToFormatSpecifier<rawType>();
    }
}

template <class... Ts> constexpr auto createFormatString()
{
    std::array<char, (typeToFormatSpecifier<Ts>().size() + ... + 1)> fmt{0};
    size_t i{0};
    for (const auto& s : std::array{typeToFormatSpecifier<Ts>()...})
    {
        for (auto c : s)
            fmt[i++] = c;
    }
    return fmt;
}

template <class... Ts> size_t printv(char* buffer, size_t max, Ts&&... args)
{
    constexpr auto fmt{createFormatString<Ts...>()};
    return std::snprintf(buffer, max, fmt.data(), std::forward<Ts>(args)...);
}
template <Log::Level level, class... Ts> void Log::print(Ts&&... args)
{
    if (level < this->level)
        return;
    time_t ctime{time(nullptr)};
    tm* time{gmtime(&ctime)};
    size_t size{printPreamble<level>(*time)};
    size += printv(&buffer[size], sizeof(buffer) - size, std::forward<Ts>(args)...);
    buffer[size] = '\n';
    size++;
    if (fileEnabled)
        file.push(buffer, size);
    if (serial != nullptr)
        serial->write(buffer, size);
}
