template <Log::Level level> size_t Log::printPreamble(const tm& time)
{
    constexpr size_t timeLength{sizeof("[0000-00-00 00:00:00]")};
    strftime(buffer, timeLength, "[%F %T]", &time);
    constexpr std::string_view levelString{levelToString(level)};
    strcpy(&buffer[timeLength - 1], levelString.cbegin());
    return timeLength + levelString.size() - 1;
}

// TODO: Clean up the type to format specifier compile-time functionality with proper SFINAE. This
// here works, but it's messy, requires two functions, and is unruly to extend.

template <class T> constexpr std::string_view Log::typeToFormatSpecifier()
{
    using rawType = std::decay_t<T>;
    if constexpr (std::is_enum_v<rawType>)
    {
        using rawType = std::underlying_type_t<rawType>;
    }
    if constexpr (std::is_same_v<std::make_unsigned_t<rawType>, unsigned char>)
    {
        if constexpr (std::is_signed_v<rawType>)
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
        return rawTypeToFormatSpecifier<rawType>();
    }
}

template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<const char*>() { return "%s"; }
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<char*>() { return "%s"; }
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<signed int>() { return "%i"; }
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<unsigned int>()
{
    return "%u";
}
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<long signed int>()
{
    return "%i";
}
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<signed char>() { return "%i"; }
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<unsigned char>()
{
    return "%u";
}
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<float>() { return "%f"; }
template <> constexpr std::string_view Log::rawTypeToFormatSpecifier<char>() { return "%c"; }

template <class... Ts> constexpr auto Log::createFormatString()
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

template <class... Ts> size_t Log::printv(char* buffer, size_t max, Ts&&... args)
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
