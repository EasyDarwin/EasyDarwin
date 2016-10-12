#pragma once
class NonCopyable
{
public:
    NonCopyable(const NonCopyable&) = delete; // deleted
    NonCopyable& operator = (const NonCopyable&) = delete; // deleted
    NonCopyable() = default;   // available
};

