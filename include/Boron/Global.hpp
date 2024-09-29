#ifndef BORON_INCLUDE_BORON_GLOBAL_HPP_
#define BORON_INCLUDE_BORON_GLOBAL_HPP_

#define BORON_VERSION_MAJOR 0
#define BORON_VERSION_MINOR 1
#define BORON_VERSION_PATCH 0
#define BORON_VERSION_STRING "0.1.0"

#define BORON_NODISCARD [[nodiscard]]

#if __has_cpp_attribute(gnu::malloc)
#define BORON_MALLOCLIKE [[nodiscard, gnu::malloc]]
#else
#define BORON_MALLOCLIKE [[nodiscard]]
#endif

#ifdef __SIZEOF_POINTER__
#define BORON_POINTER_SIZE __SIZEOF_POINTER__
#else
#define BORON_POINTER_SIZE 8
#endif

#define BORON_EXPORT

#endif