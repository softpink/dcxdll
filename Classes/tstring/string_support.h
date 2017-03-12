#pragma once
// support functions for TString & c-string handling...

#include <tchar.h>
#include <stdlib.h>

// determine whether _Ty is a Number type (excluding char / wchar)
template<class _Ty>
struct is_Numeric
	: std::integral_constant<bool,
	std::is_arithmetic_v<_Ty>
	&& !std::is_same_v<_Ty, wchar_t>
	&& !std::is_same_v<_Ty, char>
	&& !std::is_pointer_v<_Ty>
	>
{
};
template <typename T>
constexpr bool is_Numeric_v = is_Numeric<T>::value;

namespace details {
	template <typename Result, typename Format>
	Result &_ts_printf_do(Result &res, const Format &fmt)
	{
		res += fmt;
		return res;
	}

	template <typename Result, typename Format, typename Value, typename... Arguments>
	Result &_ts_printf_do(Result &res, const Format &fmt, const Value val, Arguments&&... args)
	{
		auto i = 0U;
		auto bSkip = false;
		for (auto c = fmt[0]; c != decltype(c){'\0'}; c = fmt[++i])
		{
			if (!bSkip)
			{
				if (c == decltype(c){'\\'})
				{
					bSkip = true;
					continue;
				}
				if (c == decltype(c){'%'})
				{
					res += val;
					return _ts_printf_do(res, fmt + i + 1, args...);
				}
			}
			else
				bSkip = false;
			res += c;
		}
		return res;
	}

	template<typename T, class TR = std::char_traits<T> >
	struct impl_strstr {
		using ptype = typename TR::char_type*;
		using cptype = typename const ptype;

		ptype operator()(ptype input, cptype find) {
			do {
				cptype p, q;
				for (p = input, q = find; !TR::eq(*q, 0) && TR::eq(*p, *q); p++, q++) {}

				if (TR::eq(*q, 0))
					return input;

			} while (!TR::eq(*(input++), 0));
			return nullptr;
		}
	};

	// Test if a string is empty, works for std::basic_string & TString objects
	template <typename T>
	std::enable_if_t<!std::is_pointer<T>::value && !std::is_pod<T>::value && std::is_member_function_pointer<decltype(&T::empty)>::value, bool> _ts_isEmpty(const T &str) noexcept
	{
		return str.empty();
	}

	// Test if a string is empty, works for C String char * or wchar_t *
	template <typename T>
	std::enable_if_t<std::is_pointer<T>::value, bool> _ts_isEmpty(const T &str) noexcept
	{
		using value_type = std::remove_cv_t<std::remove_pointer_t<T> >;

		static_assert(std::is_same_v<value_type, char> || std::is_same_v<value_type, wchar_t>, "Invalid Type used");
		return ((str == nullptr) || (str[0] == value_type()));
	}

	// Test if a string is empty, works for C char or wchar_t
	template <typename T>
	std::enable_if_t<!std::is_pointer_v<T> && std::is_pod_v<T>, bool> _ts_isEmpty(const T &str) noexcept
	{
		using value_type = std::remove_cv_t<T>;

		static_assert(std::is_same_v<value_type, char> || std::is_same_v<value_type, wchar_t>, "Invalid Type used");
		return (str == value_type());
	}

	// Get String length
	template <typename T, typename size_type = std::size_t>
	std::enable_if_t<std::is_pointer_v<T>, size_type> _ts_strlen(const T &str) noexcept
	{
		using value_type = std::remove_cv_t<std::remove_pointer_t<T> >;

		static_assert(std::is_same_v<value_type, char> || std::is_same_v<value_type, wchar_t>, "Invalid Type used");
		auto iLen = size_type();
		for (auto p = str; p && *p; ++p)
			++iLen;
		return iLen;
	}

	template <typename T, typename size_type = std::size_t>
	constexpr inline std::enable_if_t<!std::is_pointer_v<T> && std::is_same_v<std::remove_cv_t<T>, wchar_t>, size_type> _ts_strlen(const T &str) noexcept
	{
		return (str == T() ? 0U : 1U);
	}

	template <typename T, typename size_type = std::size_t>
	constexpr inline std::enable_if_t<!std::is_pointer_v<T> && std::is_same_v<std::remove_cv_t<T>, char>, size_type> _ts_strlen(const T &str) noexcept
	{
		return (str == T() ? 0U : 1U);
	}

	template <typename T, typename size_type = std::size_t, std::size_t N>
	constexpr inline size_type _ts_strlen(T const (&)[N]) noexcept
	{
		return (N == 0U ? 0U : N - 1);
	}

	template <typename T, typename size_type = T::size_type>
	inline std::enable_if_t<!std::is_pointer_v<T> && std::is_member_function_pointer<decltype(&T::length)>::value, size_type> _ts_strlen(const T &str)
	{
		return str.length();
	}

	template <typename T>
	struct _impl_strnlen {
	};
	template <>
	struct _impl_strnlen<char *> {
		size_t operator()(char *const ptr, size_t length) noexcept
		{
			return strnlen(ptr, length);
		}
	};
	template <>
	struct _impl_strnlen<wchar_t *> {
		size_t operator()(wchar_t *const ptr, size_t length) noexcept
		{
			return wcsnlen(ptr, length);
		}
	};
	template <>
	struct _impl_strnlen<char> {
		size_t operator()(char &ptr, size_t length) noexcept
		{
			return (ptr == 0 || length == 0 ? 0U : 1U);
		}
	};
	template <>
	struct _impl_strnlen<wchar_t> {
		size_t operator()(wchar_t &ptr, size_t length) noexcept
		{
			return (ptr == 0 || length == 0 ? 0U : 1U);
		}
	};

	template <typename T>
	struct _impl_strcpyn {
	};
	template <>
	struct _impl_strcpyn<char> {
		char *operator()(char *const pDest, const char *const pSrc, const size_t length) noexcept
		{
			//return strncpy(pDest, pSrc, length);	// doesn't guarantee NULL termination!
			return lstrcpynA(pDest, pSrc, static_cast<int>(length));
		}
	};
	template <>
	struct _impl_strcpyn<wchar_t> {
		wchar_t *operator()(wchar_t *const pDest, const wchar_t *const pSrc, const size_t length) noexcept
		{
			//return wcsncpy(pDest, pSrc, length); // doesn't guarantee NULL termination!
			return lstrcpynW(pDest, pSrc, static_cast<int>(length));
		}
	};

	template <typename T>
	struct _impl_strcpy {
	};
	template <>
	struct _impl_strcpy<char> {
		char *operator()(char *const pDest, const char *const pSrc) noexcept
		{
			return strcpy(pDest, pSrc);
		}
	};
	template <>
	struct _impl_strcpy<wchar_t> {
		wchar_t *operator()(wchar_t *const pDest, const wchar_t *const pSrc) noexcept
		{
			return wcscpy(pDest, pSrc);
		}
	};

	template <typename T>
	struct _impl_strncat {
	};
	template <>
	struct _impl_strncat<char> {
		char *operator()(char *const pDest, const char *const pSrc, const size_t length) noexcept
		{
			return strncat(pDest, pSrc, length);
		}
	};
	template <>
	struct _impl_strncat<wchar_t> {
		wchar_t *operator()(wchar_t *const pDest, const wchar_t *const pSrc, const size_t length) noexcept
		{
			return wcsncat(pDest, pSrc, length);
		}
	};

	template <typename T>
	struct _impl_strcat {
	};
	template <>
	struct _impl_strcat<char> {
		char *operator()(char *const pDest, const char *const pSrc) noexcept
		{
			return strcat(pDest, pSrc);
		}
	};
	template <>
	struct _impl_strcat<wchar_t> {
		wchar_t *operator()(wchar_t *const pDest, const wchar_t *const pSrc) noexcept
		{
			return wcscat(pDest, pSrc);
		}
	};

	template <typename T>
	struct _impl_strncmp {
	};
	template <>
	struct _impl_strncmp<char> {
		int operator()(const char *const pDest, const char *const pSrc, const size_t length) noexcept
		{
			return strncmp(pDest, pSrc, length);
		}
	};
	template <>
	struct _impl_strncmp<wchar_t> {
		int operator()(const wchar_t *const pDest, const wchar_t *const pSrc, const size_t length) noexcept
		{
			return wcsncmp(pDest, pSrc, length);
		}
	};

	template <typename T>
	struct _impl_strnicmp {
	};
	template <>
	struct _impl_strnicmp<char> {
		int operator()(const char *const pDest, const char *const pSrc, const size_t length) noexcept
		{
			return _strnicmp(pDest, pSrc, length);
		}
	};
	template <>
	struct _impl_strnicmp<wchar_t> {
		int operator()(const wchar_t *const pDest, const wchar_t *const pSrc, const size_t length) noexcept
		{
			return _wcsnicmp(pDest, pSrc, length);
		}
	};

	template <typename T>
	struct _impl_strcmp {
	};
	template <>
	struct _impl_strcmp<char> {
		int operator()(const char *const pDest, const char *const pSrc) noexcept
		{
			return strcmp(pDest, pSrc);
		}
	};
	template <>
	struct _impl_strcmp<wchar_t> {
		int operator()(const wchar_t *const pDest, const wchar_t *const pSrc) noexcept
		{
			return wcscmp(pDest, pSrc);
		}
	};

	template <typename T>
	struct _impl_stricmp {
	};
	template <>
	struct _impl_stricmp<char> {
		int operator()(const char *const pDest, const char *const pSrc) noexcept
		{
			return _stricmp(pDest, pSrc);
		}
	};
	template <>
	struct _impl_stricmp<wchar_t> {
		int operator()(const wchar_t *const pDest, const wchar_t *const pSrc) noexcept
		{
			return _wcsicmp(pDest, pSrc);
		}
	};

	template <typename T>
	struct _impl_ts_find {
	};
	template <>
	struct _impl_ts_find<char> {
		const char *operator()(const char *const str, const char &srch)
		{
			return strchr(str, srch);
		}
	};
	template <>
	struct _impl_ts_find<const char> {
		const char *operator()(const char *const str, const char &srch)
		{
			return strchr(str, srch);
		}
	};
	template <>
	struct _impl_ts_find<char *> {
		const char *operator()(const char *const str, const char *srch)
		{
			return strstr(str, srch);
		}
	};
	template <>
	struct _impl_ts_find<const char *> {
		const char *operator()(const char *const str, const char *srch)
		{
			return strstr(str, srch);
		}
	};
	template <>
	struct _impl_ts_find<wchar_t> {
		const wchar_t *operator()(const wchar_t *const str, const wchar_t &srch)
		{
			return wcschr(str, srch);
		}
	};
	template <>
	struct _impl_ts_find<const wchar_t> {
		const wchar_t *operator()(const wchar_t *const str, const wchar_t &srch)
		{
			return wcschr(str, srch);
		}
	};
	template <>
	struct _impl_ts_find<wchar_t *> {
		const wchar_t *operator()(const wchar_t *const str, const wchar_t *srch)
		{
			return wcsstr(str, srch);
		}
	};
	template <>
	struct _impl_ts_find<const wchar_t *> {
		const wchar_t *operator()(const wchar_t *const str, const wchar_t *srch)
		{
			return wcsstr(str, srch);
		}
	};

	template <typename T>
	struct _impl_vscprintf {
	};
	template <>
	struct _impl_vscprintf<char> {
		const int operator()(const char *const fmt, const va_list args)
		{
			return _vscprintf(fmt, args);
		}
	};
	template <>
	struct _impl_vscprintf<wchar_t> {
		const int operator()(const wchar_t *const fmt, const va_list args)
		{
			return _vscwprintf(fmt, args);
		}
	};

	template <typename T>
	struct _impl_vsprintf {
	};
	template <>
	struct _impl_vsprintf<char> {
		const int operator()(char *const buf, size_t nCount, const char *const fmt, const va_list args)
		{
			return vsnprintf(buf, nCount, fmt, args);
		}
	};
	template <>
	struct _impl_vsprintf<wchar_t> {
		const int operator()(wchar_t *const buf, size_t nCount, const wchar_t *const fmt, const va_list args)
		{
			return vswprintf(buf, nCount, fmt, args);
		}
	};

	template <typename T>
	struct _impl_atoi {
	};
	template <>
	struct _impl_atoi<char> {
		auto operator()(const char *const buf)
		{
			return atoi(buf);
		}
	};
	template <>
	struct _impl_atoi<wchar_t> {
		auto operator()(const wchar_t *const buf)
		{
			return _wtoi(buf);
		}
	};

	template <typename T>
	struct _impl_atoi64 {
	};
	template <>
	struct _impl_atoi64<char> {
		auto operator()(const char *const buf)
		{
			return _atoi64(buf);
		}
	};
	template <>
	struct _impl_atoi64<wchar_t> {
		auto operator()(const wchar_t *const buf)
		{
			return _wtoi64(buf);
		}
	};

	template <typename T>
	struct _impl_atof {
	};
	template <>
	struct _impl_atof<char> {
		auto operator()(const char *const buf)
		{
			return atof(buf);
		}
	};
	template <>
	struct _impl_atof<wchar_t> {
		auto operator()(const wchar_t *const buf)
		{
			return _wtof(buf);
		}
	};
}

// Check string bounds, make sure dest is not within the source string & vice versa (this could be a possible reason for some strcpyn() fails we see)
template <typename T>
constexpr bool isInBounds(const T *const sDest, const T *const sSrc, std::size_t iLen)
{
	return (sSrc > sDest && sSrc <= (sDest + iLen)) || (sDest > sSrc && sDest <= (sSrc + iLen));
}

template <typename Result, typename Format, typename Value, typename... Arguments>
Result &_ts_sprintf(Result &res, const Format &fmt, const Value val, Arguments&&... args)
{
	res.clear();
	return details::_ts_printf_do(res, fmt, val, args...);
}

template <class T>
T *_ts_strstr(T *input, const std::remove_const_t<T> *find)
{
	return details::impl_strstr<T>()(input, find);
}

#define TSTRING_WILDT 0
#define TSTRING_WILDA 0
#define TSTRING_WILDE 0
#define TSTRING_WILDW 0

template <typename TameString, typename WildString>
bool _ts_WildcardMatch(const TameString &pszString, const WildString &pszMatch) noexcept
{
	if ((pszMatch == nullptr) || (pszString == nullptr))
		return false;

	ptrdiff_t MatchPlaceholder = 0;
	ptrdiff_t StringPlaceholder = 0;
	ptrdiff_t iWildOffset = 0;
	ptrdiff_t iTameOffset = 0;

	while (pszString[iTameOffset])
	{
		if (pszMatch[iWildOffset] == TEXT('*'))
		{
			if (pszMatch[++iWildOffset] == TEXT('\0'))
				return true;
			MatchPlaceholder = iWildOffset;
			StringPlaceholder = iTameOffset + 1;
		}
#if TSTRING_WILDT
		else if (pszMatch[iWildOffset] == TEXT('~') && pszString[iTameOffset] == TEXT(' '))
		{
			++iWildOffset;
			while (pszString[iTameOffset] == TEXT(' '))
				++iTameOffset;
		}
#endif
#if TSTRING_WILDA
		else if (pszMatch[iWildOffset] == TEXT('^'))
		{
			++iWildOffset;
			if (_toupper(pszMatch[iWildOffset]) == _toupper(pszString[iTameOffset]))
				++iTameOffset;
			++iWildOffset;
		}
#endif
#if TSTRING_WILDE
		else if (pszMatch[iWildOffset] == TEXT('\\'))
		{
			// any character following a '\' is taken as a literal character.
			++iWildOffset;
			if (_toupper(pszMatch[iWildOffset]) != _toupper(pszString[iTameOffset]))
				return false;
			++iTameOffset;
		}
#endif
#if TSTRING_WILDW
		else if (pszMatch[iWildOffset] == TEXT('#'))
		{
			++iWildOffset;
			while (pszString[iTameOffset] && (pszString[iTameOffset] != TEXT(' ') || pszString[iTameOffset] != TEXT('\t') || pszString[iTameOffset] != TEXT('\n') || pszString[iTameOffset] != TEXT('\r')))
				++iTameOffset;
		}
#endif
		else if (pszMatch[iWildOffset] == TEXT('?') || _toupper(pszMatch[iWildOffset]) == _toupper(pszString[iTameOffset]))
		{
			++iWildOffset;
			++iTameOffset;
		}
		else if (StringPlaceholder == 0)
			return false;
		else
		{
			iWildOffset = MatchPlaceholder;
			iTameOffset = StringPlaceholder++;
		}
	}

	while (pszMatch[iWildOffset] == TEXT('*'))
		++iWildOffset;

	return (pszMatch[iWildOffset] == TEXT('\0'));
}

// Get String length
template <typename T>
inline auto _ts_strlen(const T &str)
{
	return details::_ts_strlen(str);
}

// Get String length (with buffer size limit)
template <typename T>
inline size_t _ts_strnlen(const T &str, const size_t &nBufSize)
{
	return details::_impl_strnlen<T>()(str, nBufSize);
}

// Test if a string is empty, works for any object that has an empty() member function as well as C strings.
template <typename T>
inline bool _ts_isEmpty(const T &str) noexcept
{
	return details::_ts_isEmpty(str);
}

// Finds a string or character within a string & returns a pointer to it.
template <typename HayStack, typename Needle>
inline HayStack *_ts_find(HayStack *const str, Needle srch)
{
	return const_cast<HayStack *>(details::_impl_ts_find<Needle>()(str, srch));
}
template <typename HayStack, typename Needle>
inline const HayStack *_ts_find(const HayStack *const str, Needle srch)
{
	return details::_impl_ts_find<Needle>()(str, srch);
}

// Finds a string literal within a string & returns a pointer to it.
template <typename T, size_t N>
inline T *_ts_find(T *const str, const T(&srch)[N])
{
	return const_cast<T *>(details::_impl_ts_find<std::add_pointer_t<T>>()(str, &srch[0]));
}

//// Finds a string or character within a string & returns a pointer to it.
//template <typename T, typename TStr>
//inline TStr *_ts_find(TStr *const str, T &srch)
//{
//	return const_cast<TStr *>(details::_impl_ts_find<std::remove_cv_t<T>>()(str, srch));
//}
//
//// Finds a string literal within a string & returns a pointer to it.
//template <typename T, typename TStr, size_t N>
//inline TStr *_ts_find(TStr *const str, const T (&srch)[N])
//{
//	return const_cast<TStr *>(details::_impl_ts_find<std::add_pointer_t<T>>()(str, &srch[0]));
//}

//template <typename T, typename TStr>
//inline const TStr *_ts_find(const TStr *const str, const T &srch)
//{
//	return details::_impl_ts_find<std::remove_cv_t<T>>()(str, srch);
//}
//
//template <typename T, typename TStr>
//inline TStr *_ts_find(TStr *const str, const T &srch)
//{
//	return const_cast<TStr *>(details::_impl_ts_find<std::remove_cv_t<T>>()(str, srch));
//}

template <typename T>
T *_ts_strcpyn(T *const sDest, const T *const sSrc, const size_t iChars) noexcept
{
	static_assert(std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Only char & wchar_t supported...");

	if ((sDest == nullptr) || (sSrc == nullptr) || isInBounds<std::remove_cv_t<T> >(sDest, sSrc, iChars))
		return nullptr;
	
	return details::_impl_strcpyn<T>()(sDest, sSrc, iChars);
}

template <typename T>
T *_ts_strcpy(T *const sDest, const T *const sSrc) noexcept
{
	static_assert(std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Only char & wchar_t supported...");

	if ((sDest == nullptr) || (sSrc == nullptr))
		return nullptr;

	return details::_impl_strcpy<T>()(sDest, sSrc);
}

template <typename T>
T *_ts_strncat(T *const sDest, const T *const sSrc, const size_t iChars) noexcept
{
	static_assert(std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Only char & wchar_t supported...");

	return details::_impl_strncat<T>()(sDest, sSrc, iChars);
}

template <typename T>
T *_ts_strcat(T *const sDest, const T *const sSrc) noexcept
{
	static_assert(std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Only char & wchar_t supported...");

	return details::_impl_strcat<T>()(sDest, sSrc);
}

template <typename T>
int _ts_strncmp(const T *const sDest, const T *const sSrc, const size_t iChars) noexcept
{
	static_assert(std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Only char & wchar_t supported...");

	return details::_impl_strncmp<T>()(sDest, sSrc, iChars);
}

template <typename T>
int _ts_strnicmp(const T *const sDest, const T *const sSrc, const size_t iChars) noexcept
{
	static_assert(std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Only char & wchar_t supported...");

	return details::_impl_strnicmp<T>()(sDest, sSrc, iChars);
}

template <typename T>
int _ts_strcmp(const T *const sDest, const T *const sSrc) noexcept
{
	static_assert(std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Only char & wchar_t supported...");

	return details::_impl_strcmp<T>()(sDest, sSrc);
}

template <typename T>
int _ts_stricmp(const T *const sDest, const T *const sSrc) noexcept
{
	static_assert(std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Only char & wchar_t supported...");

	return details::_impl_stricmp<T>()(sDest, sSrc);
}

template <typename T>
int _ts_vscprintf(const T *const _Format, va_list _ArgList)
{
	static_assert(std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Only char & wchar_t supported...");

	return details::_impl_vscprintf<T>()(_Format, _ArgList);
}

template <typename T>
int _ts_vsprintf(T *const buf, size_t nCount, const T *const fmt, const va_list args)
{
	static_assert(std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Only char & wchar_t supported...");

	return details::_impl_vsprintf<T>()(buf, nCount, fmt, args);
}

template <typename T>
auto _ts_atoi(const T *const buf)
{
	static_assert(std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Only char & wchar_t supported...");

	return details::_impl_atoi<T>()(buf);
}

template <typename T>
auto _ts_atoi64(const T *const buf)
{
	static_assert(std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Only char & wchar_t supported...");

	return details::_impl_atoi64<T>()(buf);
}

template <typename T>
auto _ts_atof(const T *const buf)
{
	static_assert(std::is_same_v<T, char> || std::is_same_v<T, wchar_t>, "Only char & wchar_t supported...");

	return details::_impl_atof<T>()(buf);
}