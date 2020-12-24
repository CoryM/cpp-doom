#// Originally based on https://en.cppreference.com/w/cpp/feature_test
#ifndef BUILD_INFO_COMPILER_FEATURE_HPP
#define BUILD_INFO_COMPILER_FEATURE_HPP

#define COMPILER_FEATURE_VALUE(value) #value
#define COMPILER_FEATURE_ENTRY(name)  { #name, COMPILER_FEATURE_VALUE(name) },

#ifdef __has_cpp_attribute
#define COMPILER_ATTRIBUTE_VALUE_AS_STRING(s) #s
#define COMPILER_ATTRIBUTE_AS_NUMBER(x)       COMPILER_ATTRIBUTE_VALUE_AS_STRING(x)
#define COMPILER_ATTRIBUTE_ENTRY(attr) \
    { #attr, COMPILER_ATTRIBUTE_AS_NUMBER(__has_cpp_attribute(attr)) },
#else
#define COMPILER_ATTRIBUTE_ENTRY(attr) { #attr, "_" },
#endif


struct CompilerFeature {
    CompilerFeature(const char *name = nullptr, const char *value = nullptr)
        : name(name)
        , value(value)
    {
    }
    const char *name;
    const char *value;
};


#endif //  BUILD_INFO_COMPILER_FEATURE_HPP