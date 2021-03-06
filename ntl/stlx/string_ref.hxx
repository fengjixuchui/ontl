/**\file*********************************************************************
 *                                                                     \brief
 *  String reference classes [strings.string_ref]
 *
 ****************************************************************************
 */
#ifndef NTL__STLX_STRINGREF
#define NTL__STLX_STRINGREF
#pragma once

#include "stdstring.hxx"

namespace std
{
  template<typename charT, typename traits = char_traits<charT> >
  class basic_string_ref;

  typedef basic_string_ref<char>      string_ref,     string_view;
  typedef basic_string_ref<char16_t>  u16string_ref,  u16string_view;
  typedef basic_string_ref<char32_t>  u32string_ref,  u32string_view;
  typedef basic_string_ref<wchar_t>   wstring_ref,    wstring_view;


  // numeric conversions
  int stoi(const string_ref& str, size_t* idx=0, int base=10);
  long stol(const string_ref& str, size_t* idx=0, int base=10);
  unsigned long stoul(const string_ref& str, size_t* idx=0, int base=10);
  long long stoll(const string_ref& str, size_t* idx=0, int base=10);
  unsigned long long stoull(const string_ref& str, size_t* idx=0, int base=10);
  float stof(const string_ref& str, size_t* idx=0);
  double stod(const string_ref& str, size_t* idx=0);
  long double stold(const string_ref& str, size_t* idx=0);

  int stoi(const wstring_ref& str, size_t* idx=0, int base=10);
  long stol(const wstring_ref& str, size_t* idx=0, int base=10);
  unsigned long stoul(const wstring_ref& str, size_t* idx=0, int base=10);
  long long stoll(const wstring_ref& str, size_t* idx=0, int base=10);
  unsigned long long stoull(const wstring_ref& str, size_t* idx=0, int base=10);
  float stof(const wstring_ref& str, size_t* idx=0);
  double stod(const wstring_ref& str, size_t* idx=0);
  long double stold(const wstring_ref& str, size_t* idx=0);


  template<typename charT, typename traits>
  class basic_string_ref
  {
    static const charT zero_char = 0;
  public:
    ///\name types
    typedef traits traits_type;
    typedef charT  value_type;
    typedef const charT* pointer;
    typedef const charT& reference;
    typedef const charT& const_reference;
    typedef pointer const_iterator;
    typedef const_iterator iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef const_reverse_iterator reverse_iterator;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    static constexpr const size_type npos = size_type(-1);

    ///\name construct/copy
    constexpr basic_string_ref()
      :p(), len()
    {}

    constexpr basic_string_ref(const basic_string_ref& r)
      :p(r.p), len(r.len)
    {}

    basic_string_ref(const charT* str)
      :p(str), len(traits::length(str))
    {}
    
    template<typename Allocator>
    basic_string_ref(const basic_string<charT, traits, Allocator>& str)
      :p(str.data()), len(str.length())
    {}

    constexpr basic_string_ref(const basic_string_ref& r, size_type pos, size_type n = npos)
      :p(r.p+pos), len(n == npos ? (r.len-pos) : n)
    {}

    constexpr basic_string_ref(const charT* str, size_type len)
      :p(str), len(len)
    {}

    basic_string_ref(initializer_list<charT> il)
      :p(il.begin()), len(il.size())
    {}
    
    basic_string_ref& operator=(const basic_string_ref& r)
    {
      p = r.p,
        len = r.len;
      return *this;
    }

    template<typename Allocator>
#ifdef NTL__CXX_EXPLICITOP
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER > 170051025 // not work with Nov'12 CTP
    explicit
#endif
#endif
      operator basic_string<charT, traits, Allocator>() const { return basic_string<charT, traits, Allocator>(p, len); }


    // iterators
    constexpr const_iterator begin() const  { return p; }
    constexpr const_iterator end() const    { return p+len; }
    constexpr const_iterator cbegin() const { return p; }
    constexpr const_iterator cend() const   { return p+len; }
    const_reverse_iterator rbegin() const   { return p+len; }
    const_reverse_iterator rend() const     { return p; }
    const_reverse_iterator crbegin() const  { return p+len; }
    const_reverse_iterator crend() const    { return p; }

    // capacity
    constexpr size_type size() const        { return len; }
    constexpr size_type length() const      { return len; }
    constexpr size_type max_size() const    { return len; }
    constexpr bool empty() const            { return len == 0; }

    // element access
    constexpr const charT& operator[](size_type pos) const
    {
      if(pos < len)
        return p[pos];
      return zero_char;
      //return pos < len ? p[pos] : zero_char;
    }
    const_reference at(size_t pos) const
    {
      if(pos >= len)
        __throw_out_of_range(__name__": pos > size()");
      return operator[](pos);
    }
    constexpr const_reference front() const    { return p[0]; }
    constexpr const_reference back() const     { return p[len-1]; }
    constexpr const charT*    data() const     { return p; }

    // modifiers
    void clear()
    {
      len = 0;
    }
    void remove_prefix(size_type n)
    {
      if(n > len)
        n = len;
      p += n,
        len -= n;
    }
    void remove_suffix(size_type n)
    {
      if(n > len)
        n = len;
      len -= n;
    }

    // basic_string_ref string operations
    constexpr basic_string_ref substr(size_type pos, size_type n=npos) const
    {
      return basic_string_ref(p+pos, n == npos ? (len-pos) : n);
    }

    int compare(const basic_string_ref& str) const
    {
      const size_t rlen = std::min(size(), str.size());
      if(!rlen)
        return static_cast<int>(size() - str.size());
      const int r = traits_type::compare(begin(), str.begin(), rlen);
      return r != 0 ? r : static_cast<int>(size() - str.size());
    }
    int compare(size_type pos, size_type n, const basic_string_ref& str) const
    {
      if(pos+n > size())
        return -1;
      const size_t rlen = std::min(n, str.size());
      if(!rlen)
        return static_cast<int>(n - str.size());
      const int r = traits_type::compare(begin()+pos, str.begin(), rlen);
      return r != 0 ? r : static_cast<int>(n - str.size());
    }

    bool starts_with(const basic_string_ref& x) const
    {
      if(x.len > len)
        return false;
      return traits_type::compare(begin(), x.begin(), x.len) == 0;
    }
    bool ends_with(const basic_string_ref& x) const
    {
      const size_type pos = x.len;
      if(x.len > len)
        return false;
      return traits_type::compare(end()-pos, x.end()-pos, pos) == 0;
    }

    //////////////////////////////////////////////////////////////////////////
    size_type find(const basic_string_ref& s, size_type pos = 0) const
    {
      const charT* const beg = begin(), *p = s.begin();
      for(size_type xpos = pos, n = s.length(); xpos + n <= len; ++xpos)
      {
        for(size_type i = 0; i != n; ++i){
          if ( !traits_type::eq(*(beg + xpos  + i), *(p + i)) )
            goto next_xpos;
        }
        return xpos;
      next_xpos:;
      }
      return npos;
    }

    size_type find(charT c, size_type pos = 0) const
    {
      if(pos >= len)
        return npos;
      pointer f = p+pos;
      const charT* e = traits_type::find(f, len-pos, c);
      return e == nullptr ? npos : (e-p);
    }

    size_type rfind(const basic_string_ref& s) const
    {
      const size_type cursize = size(), pos = 0, n = s.size();
      if(!n) return min(pos,cursize);
      size_type & xpos = pos;
      if ( xpos > cursize || xpos + n > cursize )
        xpos = cursize - n;
      const charT* const beg = begin();
      while ( xpos + n > 0 )
      {
        for ( size_type i = 0; i != n; ++i )
          if ( !traits_type::eq(*(beg + xpos + i), *(s + i)) )
            goto next_xpos;
        return xpos;
      next_xpos:
        --xpos;
      }
      return npos;
    }

    size_type rfind(charT c) const
    {
      size_type xpos = size();
      const charT* const beg = begin();
      while ( xpos-- )
        if ( traits_type::eq(*(beg + xpos), c) )
          return xpos;
      return npos;
    }

    size_type find_first_of(const basic_string_ref& s) const;
    size_type find_first_of(charT c) const;
    size_type find_last_of(const basic_string_ref& s) const;
    size_type find_last_of(charT c) const;
    size_type find_first_not_of(const basic_string_ref& s) const;
    size_type find_first_not_of(charT c) const;
    size_type find_last_not_of(const basic_string_ref& s) const;
    size_type find_last_not_of(charT c) const;

  private:
    pointer p;
    size_type len;
  };

  // Comparison operators
  template<typename charT, typename traits>
  inline bool operator == (const basic_string_ref<charT, traits>& x, const basic_string_ref<charT, traits>& y) { return x.compare(y) == 0; }
  template<typename charT, typename traits>
  inline bool operator != (const basic_string_ref<charT, traits>& x, const basic_string_ref<charT, traits>& y) { return x.compare(y) != 0; }
  template<typename charT, typename traits>
  inline bool operator <  (const basic_string_ref<charT, traits>& x, const basic_string_ref<charT, traits>& y) { return x.compare(y) <  0; }
  template<typename charT, typename traits>
  inline bool operator >  (const basic_string_ref<charT, traits>& x, const basic_string_ref<charT, traits>& y) { return x.compare(y) >  0; }
  template<typename charT, typename traits>
  inline bool operator <= (const basic_string_ref<charT, traits>& x, const basic_string_ref<charT, traits>& y) { return x.compare(y) <= 0; }
  template<typename charT, typename traits>
  inline bool operator >= (const basic_string_ref<charT, traits>& x, const basic_string_ref<charT, traits>& y) { return x.compare(y) >= 0; }

  template<class charT, class traits, class Allocator>
  inline bool operator==(const basic_string<charT,traits,Allocator>& lhs, const basic_string_ref<charT,traits>& rhs) { return basic_string_ref<charT, traits>(lhs) == rhs; }
  template<class charT, class traits, class Allocator>
  inline bool operator!=(const basic_string<charT,traits,Allocator>& lhs, const basic_string_ref<charT,traits>& rhs) { return basic_string_ref<charT, traits>(lhs) != rhs; }
  template<class charT, class traits, class Allocator>
  inline bool operator==(const basic_string_ref<charT,traits>& lhs, const basic_string<charT,traits,Allocator>& rhs) { return basic_string_ref<charT, traits>(rhs) == lhs; }
  template<class charT, class traits, class Allocator>
  inline bool operator!=(const basic_string_ref<charT,traits>& lhs, const basic_string<charT,traits,Allocator>& rhs) { return basic_string_ref<charT, traits>(rhs) != lhs; }

  template<typename charT, typename traits>
  inline bool operator == (const basic_string_ref<charT, traits>& x, const charT* y) { return x == basic_string_ref<charT, traits>(y); }
  template<typename charT, typename traits>
  inline bool operator != (const basic_string_ref<charT, traits>& x, const charT* y) { return x != basic_string_ref<charT, traits>(y); }
  template<typename charT, typename traits>
  inline bool operator == (const charT* x, const basic_string_ref<charT, traits>& y) { return y == basic_string_ref<charT, traits>(x); }
  template<typename charT, typename traits>
  inline bool operator != (const charT* x, const basic_string_ref<charT, traits>& y) { return y != basic_string_ref<charT, traits>(x); }

  // Inserter
  template<class charT, class traits>
  inline basic_ostream<charT, traits>& operator<<(basic_ostream<charT, traits>& os, const basic_string_ref<charT,traits>& str)
  {
    return __::stream_writer<charT,traits>::formatted_write(os, str.data(), str.size());
  }

  // Hashing
  template<> struct hash<string_ref>;
  template<> struct hash<u16string_ref>;
  template<> struct hash<u32string_ref>;
  template<> struct hash<wstring_ref>;

  //////////////////////////////////////////////////////////////////////////
  // string literals
  __inline_ns namespace literals
  {
    __inline_ns namespace string_literals
    {
      inline string_ref   operator "" _s(const char*    str, size_t len)  { return  string_ref(str, len); }
      inline wstring_ref  operator "" _s(const wchar_t* str, size_t len)  { return wstring_ref(str, len); }
    }
  }

#ifndef NTL_CXX_NS
  // inline namespace 
  using namespace literals;
  using namespace literals::string_literals;
#endif
}

#include "stoi.hxx"

#endif// NTL__STLX_STRINGREF
