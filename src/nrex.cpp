//  NREX: Node RegEx
//  Version 0.2
//
//  Copyright (c) 2015-2016, Zher Huei Lee
//  All rights reserved.
//
//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.
//
//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:
//
//   1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would
//      be appreciated but is not required.
//
//   2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//   3. This notice may not be removed or altered from any source
//      distribution.
//

#include ".\..\src\stdafx.h"
#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include ".\..\src\wain.h"

#include "nrex.h"
#include <climits>
#include <iostream>

#ifdef NREX_UNICODE
#include <wctype.h>
#include <wchar.h>
#define NREX_ISALPHANUM iswalnum
#define NREX_ISSPACE iswspace
#define NREX_STRLEN wcslen
#else
#include <ctype.h>
#include <string.h>
#define NREX_ISALPHANUM isalnum
#define NREX_ISSPACE isspace
#define NREX_STRLEN strlen
#endif

#ifdef NREX_THROW_ERROR
#define NREX_COMPILE_ERROR(M) throw nrex_compile_error(M)
#else
#define NREX_COMPILE_ERROR(M) do { Reset(); m_errString = M; return false; } while(0)
#endif

#ifndef NREX_NEW
#define NREX_NEW(X) new X
#define NREX_NEW_ARRAY(X, N) new X[N]
#define NREX_DELETE(X) delete X
#define NREX_DELETE_ARRAY(X) delete[] X
#endif

template<typename T>
class NRexArray
{
    private:
        T* _data;
        unsigned int _reserved;
        unsigned int _size;
    public:
        NRexArray()
            : _data(NREX_NEW_ARRAY(T, 2))
            , _reserved(2)
            , _size(0)
        {
        }

        NRexArray(unsigned int reserved)
            : _data(NREX_NEW_ARRAY(T, reserved ? reserved : 1))
            , _reserved(reserved ? reserved : 1)
            , _size(0)
        {
        }

        ~NRexArray()
        {
            NREX_DELETE_ARRAY(_data);
        }

        unsigned int size() const
        {
            return _size;
        }

        void reserve(unsigned int size)
        {
            if (size < _size) {
                size = _size;
            }
            if (size == 0) {
                size = 1;
            }
            T* old = _data;
            _data = NREX_NEW_ARRAY(T, size);
            _reserved = size;
            for (unsigned int i = 0; i < _size; ++i)
            {
                _data[i] = old[i];
            }
            NREX_DELETE_ARRAY(old);
        }

        void push(T item)
        {
            if (_size == _reserved)
            {
                reserve(_reserved * 2);
            }
            _data[_size] = item;
            _size++;
        }

        const T& top() const
        {
            return _data[_size - 1];
        }

        const T& operator[] (unsigned int i) const
        {
            return _data[i];
        }

        void pop()
        {
            if (_size > 0)
            {
                --_size;
            }
        }
};

static int nrex_parse_hex(NRexChar c)
{
    if ('0' <= c && c <= '9')
    {
        return int(c - '0');
    }
    else if ('a' <= c && c <= 'f')
    {
        return int(c - 'a') + 10;
    }
    else if ('A' <= c && c <= 'F')
    {
        return int(c - 'A') + 10;
    }
    return -1;
}

static NRexChar nrex_unescape(const NRexChar*& c)
{
    switch (c[1])
    {
        case '0': ++c; return '\0';
        case 'a': ++c; return '\a';
        case 'e': ++c; return '\x1B';
        case 'f': ++c; return '\f';
        case 'n': ++c; return '\n';
        case 'r': ++c; return '\r';
        case 't': ++c; return '\t';
        case 'v': ++c; return '\v';
        case 'b': ++c; return '\b';
        case 'x':
        {
            int point = 0;
            for (int i = 2; i <= 3; ++i)
            {
                int res = nrex_parse_hex(c[i]);
                if (res == -1)
                {
                    return '\0';
                }
                point = (point << 4) + res;
            }
            c = &c[3];
            return NRexChar(point);
        }
        case 'u':
        {
            int point = 0;
            for (int i = 2; i <= 5; ++i)
            {
                int res = nrex_parse_hex(c[i]);
                if (res == -1)
                {
                    return '\0';
                }
                point = (point << 4) + res;
            }
            c = &c[5];
            return NRexChar(point);
        }
    }
    return (++c)[0];
}

class NRexSearch
{
public:
   const NRexChar* str;
   std::vector<NRexResult> m_captures;
   int end;
   bool complete;
   NRexArray<int> lookahead_pos;

   NRexChar at(int pos)
   {
      return str[pos];
   }

   NRexSearch(const NRexChar* str, int lookahead)
    : str(str),
      end(0),
      lookahead_pos(lookahead)
   {
   }
};

class NRexNode
{
public:
   NRexNode* next;
   NRexNode* previous;
   NRexNode* parent;
   bool quantifiable;
   int length;

   NRexNode(bool quantify = false)
    : next(NULL),
      previous(NULL),
      parent(NULL),
      quantifiable(quantify),
      length(-1)
   {
   }

   virtual ~NRexNode()
   {
      if (next)
      {
         NREX_DELETE(next);
      }
   }

   virtual int test(NRexSearch* s, int pos) const
   {
      return next ? next->test(s, pos) : -1;
   }

   virtual int test_parent(NRexSearch* s, int pos) const
   {
      if (next)
      {
         pos = next->test(s, pos);
      }
      if (pos >= 0)
      {
         s->complete = true;
      }
      if (parent && pos >= 0)
      {
         pos = parent->test_parent(s, pos);
      }
      if (pos < 0)
      {
         s->complete = false;
      }
      return pos;
   }

   void increment_length(int amount, bool subtract = false)
   {
      if (amount >= 0 && length >= 0)
      {
         if (!subtract)
         {
            length += amount;
         }
         else
         {
            length -= amount;
         }
      }
      else
      {
         length = -1;
      }
      if (parent)
      {
         parent->increment_length(amount, subtract);
      }
   }
};

enum nrex_group_type
{
    nrex_group_capture,
    nrex_group_non_capture,
    nrex_group_bracket,
    nrex_group_look_ahead,
    nrex_group_look_behind
};

struct NRexNodeGroup : public NRexNode
{
   nrex_group_type type;
   unsigned m_id;
   bool negate;
   NRexArray<NRexNode*> childset;
   NRexNode* back;

   NRexNodeGroup(nrex_group_type type, int id = 0)
    : NRexNode(true),
      type(type),
      m_id(id),
      negate(false),
      back(NULL)
   {
      if (type != nrex_group_bracket)
      {
         length = 0;
      }
      else
      {
         length = 1;
      }
      if (type == nrex_group_look_ahead || type == nrex_group_look_behind)
      {
         quantifiable = false;
      }
   }

   virtual ~NRexNodeGroup()
   {
      for (unsigned int i = 0; i < childset.size(); ++i)
      {
         NREX_DELETE(childset[i]);
      }

   }

   int test(NRexSearch* s, int pos) const
   {
      if (s->m_captures.size() <= m_id)
      {
         s->m_captures.resize(m_id + 1);
      }
      int old_start;
      if (type == nrex_group_capture)
      {
         old_start = s->m_captures[m_id].start;
         s->m_captures[m_id].start = pos;
      }
      for (unsigned int i = 0; i < childset.size(); ++i)
      {
         s->complete = false;
         int offset = 0;
         if (type == nrex_group_look_behind)
         {
            if (pos < length)
            {
               return -1;
            }
            offset = length;
         }
         if (type == nrex_group_look_ahead)
         {
            s->lookahead_pos.push(pos);
         }
         int res = childset[i]->test(s, pos - offset);
         if (type == nrex_group_look_ahead)
         {
            s->lookahead_pos.pop();
         }
         if (s->complete)
         {
            return res;
         }
         if (negate)
         {
            if (res < 0)
            {
               res = pos + 1;
            }
            else
            {
               return -1;
            }
            if (i + 1 < childset.size())
            {
               continue;
            }
         }
         if (res >= 0)
         {
            if (type == nrex_group_capture)
            {
               s->m_captures[m_id].length = res - pos;
            }
            else if (type == nrex_group_look_ahead || type == nrex_group_look_behind)
            {
               res = pos;
            }
            return next ? next->test(s, res) : res;
         }
      }
      if (type == nrex_group_capture)
      {
         s->m_captures[m_id].start = old_start;
      }
      return -1;
   }

        virtual int test_parent(NRexSearch* s, int pos) const
        {
            if (type == nrex_group_capture)
            {
                s->m_captures[m_id].length = pos - s->m_captures[m_id].start;
            }
            if (type == nrex_group_look_ahead)
            {
                pos = s->lookahead_pos[m_id];
            }
            return NRexNode::test_parent(s, pos);
        }

        void add_childset()
        {
            if (childset.size() > 0 && type != nrex_group_bracket)
            {
                length = -1;
            }
            back = NULL;
        }

        void add_child(NRexNode* node)
        {
            node->parent = this;
            node->previous = back;
            if (back && type != nrex_group_bracket)
            {
                back->next = node;
            }
            else
            {
                childset.push(node);
            }
            if (type != nrex_group_bracket)
            {
                increment_length(node->length);
            }
            back = node;
        }

        NRexNode* swap_back(NRexNode* node)
        {
            if (!back)
            {
                add_child(node);
                return NULL;
            }
            NRexNode* old = back;
            if (!old->previous)
            {
                childset.pop();
            }
            if (type != nrex_group_bracket)
            {
                increment_length(old->length, true);
            }
            back = old->previous;
            add_child(node);
            return old;
        }

        void pop_back()
        {
            if (back)
            {
                NRexNode* old = back;
                if (!old->previous)
                {
                    childset.pop();
                }
                if (type != nrex_group_bracket)
                {
                    increment_length(old->length, true);
                }
                back = old->previous;
                NREX_DELETE(old);
            }
        }
};

struct NRexNodeChar : public NRexNode
{
   NRexChar ch;

   NRexNodeChar(NRexChar c)
    : NRexNode(true),
      ch(c)
   {
      length = 1;
   }

   int test(NRexSearch* s, int pos) const
   {
      if (s->end <= pos || 0 > pos || s->at(pos) != ch)
      {
         return -1;
      }
      return next ? next->test(s, pos + 1) : pos + 1;
   }
};

struct NRexNodeRange : public NRexNode
{
   NRexChar start;
   NRexChar end;

   NRexNodeRange(NRexChar s, NRexChar e)
    : NRexNode(true),
      start(s),
      end(e)
   {
      length = 1;
   }

   int test(NRexSearch* s, int pos) const
   {
      if (s->end <= pos || 0 > pos)
      {
         return -1;
      }
      NRexChar c = s->at(pos);
      if (c < start || end < c)
      {
         return -1;
      }
      return next ? next->test(s, pos + 1) : pos + 1;
   }
};

enum nrex_class_type
{
    nrex_class_none,
    nrex_class_alnum,
    nrex_class_alpha,
    nrex_class_blank,
    nrex_class_cntrl,
    nrex_class_digit,
    nrex_class_graph,
    nrex_class_lower,
    nrex_class_print,
    nrex_class_punct,
    nrex_class_space,
    nrex_class_upper,
    nrex_class_xdigit,
    nrex_class_word
};

static bool nrex_compare_class(const NRexChar** pos, const char* text)
{
    unsigned int i = 0;
    for (i = 0; text[i] != '\0'; ++i)
    {
        if ((*pos)[i] != text[i])
        {
            return false;
        }
    }
    if ((*pos)[i++] != ':' || (*pos)[i] != ']')
    {
        return false;
    }
    *pos = &(*pos)[i];
    return true;
}

#define NREX_COMPARE_CLASS(POS, NAME) if (nrex_compare_class(POS, #NAME)) return nrex_class_ ## NAME

static nrex_class_type nrex_parse_class(const NRexChar** pos)
{
    NREX_COMPARE_CLASS(pos, alnum);
    NREX_COMPARE_CLASS(pos, alpha);
    NREX_COMPARE_CLASS(pos, blank);
    NREX_COMPARE_CLASS(pos, cntrl);
    NREX_COMPARE_CLASS(pos, digit);
    NREX_COMPARE_CLASS(pos, graph);
    NREX_COMPARE_CLASS(pos, lower);
    NREX_COMPARE_CLASS(pos, print);
    NREX_COMPARE_CLASS(pos, punct);
    NREX_COMPARE_CLASS(pos, space);
    NREX_COMPARE_CLASS(pos, upper);
    NREX_COMPARE_CLASS(pos, xdigit);
    NREX_COMPARE_CLASS(pos, word);
    return nrex_class_none;
}

struct NRexNodeClass : public NRexNode
{
        nrex_class_type type;

        NRexNodeClass(nrex_class_type t)
            : NRexNode(true)
            , type(t)
        {
            length = 1;
        }

        int test(NRexSearch* s, int pos) const
        {
            if (s->end <= pos || 0 > pos)
            {
                return -1;
            }
            if (!test_class(s->at(pos)))
            {
                return -1;
            }
            return next ? next->test(s, pos + 1) : pos + 1;
        }

        bool test_class(NRexChar c) const
        {
            if ((0 <= c && c <= 0x1F) || c == 0x7F)
            {
                if (type == nrex_class_cntrl)
                {
                    return true;
                }
            }
            else if (c < 0x7F)
            {
                if (type == nrex_class_print)
                {
                    return true;
                }
                else if (type == nrex_class_graph && c != ' ')
                {
                    return true;
                }
                else if ('0' <= c && c <= '9')
                {
                    switch (type)
                    {
                        case nrex_class_alnum:
                        case nrex_class_digit:
                        case nrex_class_xdigit:
                        case nrex_class_word:
                            return true;
                        default:
                            break;
                    }
                }
                else if ('A' <= c && c <= 'Z')
                {
                    switch (type)
                    {
                        case nrex_class_alnum:
                        case nrex_class_alpha:
                        case nrex_class_upper:
                        case nrex_class_word:
                            return true;
                        case nrex_class_xdigit:
                            if (c <= 'F')
                            {
                                return true;
                            }
                        default:
                            break;
                    }
                }
                else if ('a' <= c && c <= 'z')
                {
                    switch (type)
                    {
                        case nrex_class_alnum:
                        case nrex_class_alpha:
                        case nrex_class_lower:
                        case nrex_class_word:
                            return true;
                        case nrex_class_xdigit:
                            if (c <= 'f')
                            {
                                return true;
                            }
                        default:
                            break;
                    }
                }
            }
            switch (c)
            {
                case ' ':
                case '\t':
                    if (type == nrex_class_blank)
                    {
                        return true;
                    }
                case '\r':
                case '\n':
                case '\f':
                    if (type == nrex_class_space)
                    {
                        return true;
                    }
                    break;
                case '_':
                    if (type == nrex_class_word)
                    {
                        return true;
                    }
                case ']':
                case '[':
                case '!':
                case '"':
                case '#':
                case '$':
                case '%':
                case '&':
                case '\'':
                case '(':
                case ')':
                case '*':
                case '+':
                case ',':
                case '.':
                case '/':
                case ':':
                case ';':
                case '<':
                case '=':
                case '>':
                case '?':
                case '@':
                case '\\':
                case '^':
                case '`':
                case '{':
                case '|':
                case '}':
                case '~':
                case '-':
                    if (type == nrex_class_punct)
                    {
                        return true;
                    }
                    break;
                default:
                    break;
            }
            return false;
        }
};

static bool nrex_is_shorthand(NRexChar repr)
{
    switch (repr)
    {
        case 'W':
        case 'w':
        case 'D':
        case 'd':
        case 'S':
        case 's':
            return true;
    }
    return false;
}

struct NRexNodeShorthand : public NRexNode
{
        NRexChar repr;

        NRexNodeShorthand(NRexChar c)
            : NRexNode(true)
            , repr(c)
        {
            length = 1;
        }

        int test(NRexSearch* s, int pos) const
        {
            if (s->end <= pos || 0 > pos)
            {
                return -1;
            }
            bool found = false;
            bool invert = false;
            NRexChar c = s->at(pos);
            switch (repr)
            {
                case '.':
                    found = true;
                    break;
                case 'W':
                    invert = true;
                case 'w':
                    if (c == '_' || NREX_ISALPHANUM(c))
                    {
                        found = true;
                    }
                    break;
                case 'D':
                    invert = true;
                case 'd':
                    if ('0' <= c && c <= '9')
                    {
                        found = true;
                    }
                    break;
                case 'S':
                    invert = true;
                case 's':
                    if (NREX_ISSPACE(c))
                    {
                        found = true;
                    }
                    break;
            }
            if (found == invert)
            {
                return -1;
            }
            return next ? next->test(s, pos + 1) : pos + 1;
        }
};

static bool nrex_is_quantifier(NRexChar repr)
{
    switch (repr)
    {
        case '?':
        case '*':
        case '+':
        case '{':
            return true;
    }
    return false;
}

struct NRexNodeQuantifier : public NRexNode
{
        int m_min;
        int m_max;
        bool greedy;
        NRexNode* child;

        NRexNodeQuantifier(int _min, int _max)
            : NRexNode()
            , m_min(_min)
            , m_max(_max)
            , greedy(true)
            , child(NULL)
        {
        }

        virtual ~NRexNodeQuantifier()
        {
            if (child)
            {
                NREX_DELETE(child);
            }
        }

        int test(NRexSearch* s, int pos) const
        {
            return test_step(s, pos, 0, pos);
        }

        int test_step(NRexSearch* s, int pos, int level, int start) const
        {
            if (pos > s->end)
            {
                return -1;
            }
            if (!greedy && level > m_min)
            {
                int res = pos;
                if (next)
                {
                    res = next->test(s, res);
                }
                if (s->complete)
                {
                    return res;
                }
                if (res >= 0 && parent->test_parent(s, res) >= 0)
                {
                    return res;
                }
            }
            if (m_max >= 0 && level > m_max)
            {
                return -1;
            }
            if (level > 1 && level > m_min + 1 && pos == start)
            {
                return -1;
            }
            int res = pos;
            if (level >= 1)
            {
                res = child->test(s, pos);
                if (s->complete)
                {
                    return res;
                }
            }
            if (res >= 0)
            {
                int res_step = test_step(s, res, level + 1, start);
                if (res_step >= 0)
                {
                    return res_step;
                }
                else if (greedy && level >= m_min)
                {
                    if (next)
                    {
                        res = next->test(s, res);
                    }
                    if (s->complete)
                    {
                        return res;
                    }
                    if (res >= 0 && parent->test_parent(s, res) >= 0)
                    {
                        return res;
                    }
                }
            }
            return -1;
        }

        virtual int test_parent(NRexSearch* s, int pos) const
        {
            s->complete = false;
            return pos;
        }
};

struct NRexNodeAnchor : public NRexNode
{
        bool end;

        NRexNodeAnchor(bool end)
            : NRexNode()
            , end(end)
        {
            length = 0;
        }

        int test(NRexSearch* s, int pos) const
        {
            if (!end && pos != 0)
            {
                return -1;
            }
            else if (end && pos != s->end)
            {
                return -1;
            }
            return next ? next->test(s, pos) : pos;
        }
};

struct NRexNodeWordBoundary : public NRexNode
{
        bool inverse;

        NRexNodeWordBoundary(bool inverse)
            : NRexNode()
            , inverse(inverse)
        {
            length = 0;
        }

        int test(NRexSearch* s, int pos) const
        {
            bool left = false;
            bool right = false;
            if (pos != 0)
            {
                NRexChar c = s->at(pos - 1);
                if (c == '_' || NREX_ISALPHANUM(c))
                {
                    left = true;
                }
            }
            if (pos != s->end)
            {
                NRexChar c = s->at(pos);
                if (c == '_' || NREX_ISALPHANUM(c))
                {
                    right = true;
                }
            }
            if ((left != right) == inverse)
            {
                return -1;
            }
            return next ? next->test(s, pos) : pos;
        }
};

struct NRexNodeBackReference : public NRexNode
{
   int ref;

   NRexNodeBackReference(int ref)
    : NRexNode(true),
      ref(ref)
   {
      length = -1;
   }

   int test(NRexSearch* s, int pos) const
   {
      NRexResult& r = s->m_captures[ref];
      for (int i = 0; i < r.length; ++i)
      {
         if (pos + i >= s->end)
         {
            return -1;
         }
         if (s->at(r.start + i) != s->at(pos + i))
         {
            return -1;
         }
      }
      return next ? next->test(s, pos + r.length) : pos + r.length;
   }
};

bool nrex_has_lookbehind(NRexArray<NRexNodeGroup*>& stack)
{
    for (unsigned int i = 0; i < stack.size(); i++)
    {
        if (stack[i]->type == nrex_group_look_behind)
        {
            return true;
        }
    }
    return false;
}

NRex::NRex()
 : m_capturing(0),
   m_lookaheadDepth(0),
   m_root(0)
{
}

NRex::NRex(const NRexChar* pattern, int captures)
 : m_capturing(0),
   m_lookaheadDepth(0),
   m_root(NULL)
{
   Compile(pattern, captures);
}

NRex::~NRex()
{
   if (m_root)
   {
      NREX_DELETE(m_root);
   }
}

bool NRex::Valid() const
{
    return m_root != NULL;
}

void NRex::Reset()
{
    m_capturing = 0;
    m_lookaheadDepth = 0;
    if (m_root)
    {
        NREX_DELETE(m_root);
    }
    m_root = NULL;
    m_errString = "";
}

int NRex::CaptureSize() const
{
    if (m_root)
    {
        return m_capturing + 1;
    }
    return 0;
}

bool NRex::Compile(const NRexChar* pattern, int captures)
{
   Reset();
   NRexNodeGroup* root = NREX_NEW(NRexNodeGroup(nrex_group_capture, m_capturing));
   NRexArray<NRexNodeGroup*> stack;
   stack.push(root);
   unsigned int lookahead_level = 0;
   m_root = root;

   for (const NRexChar* c = pattern; c[0] != '\0'; ++c)
   {
      if (c[0] == '(')
      {
         if (c[1] == '?')
         {
            if (c[2] == ':')
            {
               c = &c[2];
               NRexNodeGroup* group = NREX_NEW(NRexNodeGroup(nrex_group_non_capture));
               stack.top()->add_child(group);
               stack.push(group);
            }
            else if (c[2] == '!' || c[2] == '=')
            {
               c = &c[2];
               NRexNodeGroup* group = NREX_NEW(NRexNodeGroup(nrex_group_look_ahead, lookahead_level++));
               group->negate = (c[0] == '!');
               stack.top()->add_child(group);
               stack.push(group);
               if (lookahead_level > m_lookaheadDepth)
               {
                  m_lookaheadDepth = lookahead_level;
               }
            }
            else if (c[2] == '<' && (c[3] == '!' || c[3] == '='))
            {
               c = &c[3];
               NRexNodeGroup* group = NREX_NEW(NRexNodeGroup(nrex_group_look_behind));
               group->negate = (c[0] == '!');
               stack.top()->add_child(group);
               stack.push(group);
            }
            else
            {
               NREX_COMPILE_ERROR("unrecognised qualifier for group");
            }
         }
         else if (captures >= 0 && m_capturing < captures && m_capturing < INT_MAX)
         {
            NRexNodeGroup* group = NREX_NEW(NRexNodeGroup(nrex_group_capture, ++m_capturing));
            stack.top()->add_child(group);
            stack.push(group);
         }
         else
         {
            NRexNodeGroup* group = NREX_NEW(NRexNodeGroup(nrex_group_non_capture));
            stack.top()->add_child(group);
            stack.push(group);
         }
      }
      else if (c[0] == ')')
      {
         if (stack.size() > 1)
         {
            if (stack.top()->type == nrex_group_look_ahead)
            {
               --lookahead_level;
            }
            stack.pop();
         }
         else
         {
            NREX_COMPILE_ERROR("unexpected ')'");
         }
      }
      else if (c[0] == '[')
      {
         NRexNodeGroup* group = NREX_NEW(NRexNodeGroup(nrex_group_bracket));
         stack.top()->add_child(group);
         if (c[1] == '^')
         {
            group->negate = true;
            ++c;
         }
         bool first_child = true;
         NRexChar previous_child;
         bool previous_child_single = false;
         while (true)
         {
            group->add_childset();
            ++c;
            if (c[0] == '\0')
            {
               NREX_COMPILE_ERROR("unclosed bracket expression '['");
            }
            if (c[0] == '[' && c[1] == ':')
            {
               const NRexChar* d = &c[2];
               nrex_class_type cls = nrex_parse_class(&d);
               if (cls != nrex_class_none)
               {
                  c = d;
                  group->add_child(NREX_NEW(NRexNodeClass(cls)));
                  previous_child_single = false;
               }
               else
               {
                  group->add_child(NREX_NEW(NRexNodeChar('[')));
                  previous_child = '[';
                  previous_child_single = true;
               }
            }
            else if (c[0] == ']' && !first_child)
            {
               break;
            }
            else if (c[0] == '\\')
            {
               if (nrex_is_shorthand(c[1]))
               {
                  group->add_child(NREX_NEW(NRexNodeShorthand(c[1])));
                  ++c;
                  previous_child_single = false;
               }
               else
               {
                  const NRexChar* d = c;
                  NRexChar unescaped = nrex_unescape(d);
                  if (c == d)
                  {
                     NREX_COMPILE_ERROR("invalid escape token");
                  }
                  group->add_child(NREX_NEW(NRexNodeChar(unescaped)));
                  c = d;
                  previous_child = unescaped;
                  previous_child_single = true;
               }
            }
            else if (previous_child_single && c[0] == '-')
            {
               bool is_range = false;
               NRexChar next;
               if (c[1] != '\0' && c[1] != ']')
               {
                  if (c[1] == '\\')
                  {
                     const NRexChar* d = ++c;
                     next = nrex_unescape(d);
                     if (c == d)
                     {
                        NREX_COMPILE_ERROR("invalid escape token in range");
                     }
                  }
                  else
                  {
                     next = c[1];
                     ++c;
                  }
                  is_range = true;
               }
               if (is_range)
               {
                  if (next < previous_child)
                  {
                     NREX_COMPILE_ERROR("text range out of order");
                  }
                  group->pop_back();
                  group->add_child(NREX_NEW(NRexNodeRange(previous_child, next)));
                  previous_child_single = false;
               }
               else
               {
                  group->add_child(NREX_NEW(NRexNodeChar(c[0])));
                  previous_child = c[0];
                  previous_child_single = true;
               }
            }
            else
            {
               group->add_child(NREX_NEW(NRexNodeChar(c[0])));
               previous_child = c[0];
               previous_child_single = true;
            }
            first_child = false;
         }
      }
      else if (nrex_is_quantifier(c[0]))
      {
         int min = 0;
         int max = -1;
         bool valid_quantifier = true;
         if (c[0] == '?')
         {
            min = 0;
            max = 1;
         }
         else if (c[0] == '+')
         {
            min = 1;
            max = -1;
         }
         else if (c[0] == '*')
         {
            min = 0;
            max = -1;
         }
         else if (c[0] == '{')
         {
            bool max_set = false;
            const NRexChar* d = c;
            while (true)
            {
               ++d;
               if (d[0] == '\0')
               {
                  valid_quantifier = false;
                  break;
               }
               else if (d[0] == '}')
               {
                  break;
               }
               else if (d[0] == ',')
               {
                  max_set = true;
                  continue;
               }
               else if (d[0] < '0' || '9' < d[0])
               {
                  valid_quantifier = false;
                  break;
               }
               if (max_set)
               {
                  if (max < 0)
                  {
                     max = int(d[0] - '0');
                  }
                  else
                  {
                     max = max * 10 + int(d[0] - '0');
                  }
               }
               else
               {
                  min = min * 10 + int(d[0] - '0');
               }
            }
            if (!max_set)
            {
               max = min;
            }
            if (valid_quantifier)
            {
               c = d;
            }
         }
         if (valid_quantifier)
         {
            if (stack.top()->back == NULL || !stack.top()->back->quantifiable)
            {
               NREX_COMPILE_ERROR("element not quantifiable");
            }
            NRexNodeQuantifier* quant = NREX_NEW(NRexNodeQuantifier(min, max));
            if (min == max)
            {
               if (stack.top()->back->length >= 0)
               {
                  quant->length = max * stack.top()->back->length;
               }
            }
            else
            {
               if (nrex_has_lookbehind(stack))
               {
                  NREX_COMPILE_ERROR("variable length quantifiers inside lookbehind not supported");
               }
            }
            quant->child = stack.top()->swap_back(quant);
            quant->child->previous = NULL;
            quant->child->next = NULL;
            quant->child->parent = quant;
            if (c[1] == '?')
            {
               quant->greedy = false;
               ++c;
            }
         }
         else
         {
            stack.top()->add_child(NREX_NEW(NRexNodeChar(c[0])));
         }
      }
      else if (c[0] == '|')
      {
         if (nrex_has_lookbehind(stack))
         {
            NREX_COMPILE_ERROR("alternations inside lookbehind not supported");
         }
         stack.top()->add_childset();
      }
      else if (c[0] == '^' || c[0] == '$')
      {
         stack.top()->add_child(NREX_NEW(NRexNodeAnchor((c[0] == '$'))));
      }
      else if (c[0] == '.')
      {
         stack.top()->add_child(NREX_NEW(NRexNodeShorthand('.')));
      }
      else if (c[0] == '\\')
      {
         if (nrex_is_shorthand(c[1]))
         {
            stack.top()->add_child(NREX_NEW(NRexNodeShorthand(c[1])));
            ++c;
         }
         else if (('1' <= c[1] && c[1] <= '9') || (c[1] == 'g' && c[2] == '{'))
         {
            int ref = 0;
            bool unclosed = false;
            if (c[1] == 'g')
            {
               unclosed = true;
               c = &c[2];
            }
            while ('0' <= c[1] && c[1] <= '9')
            {
               ref = ref * 10 + int(c[1] - '0');
               ++c;
            }
            if (c[1] == '}')
            {
               unclosed = false;
               ++c;
            }
            if (ref > m_capturing || ref <= 0 || unclosed)
            {
               NREX_COMPILE_ERROR("backreference to non-existent capture");
            }
            if (nrex_has_lookbehind(stack))
            {
               NREX_COMPILE_ERROR("backreferences inside lookbehind not supported");
            }
            stack.top()->add_child(NREX_NEW(NRexNodeBackReference(ref)));
         }
         else if (c[1] == 'b' || c[1] == 'B')
         {
            stack.top()->add_child(NREX_NEW(NRexNodeWordBoundary(c[1] == 'B')));
            ++c;
         }
         else
         {
            const NRexChar* d = c;
            NRexChar unescaped = nrex_unescape(d);
            if (c == d)
            {
               NREX_COMPILE_ERROR("invalid escape token");
            }
            stack.top()->add_child(NREX_NEW(NRexNodeChar(unescaped)));
            c = d;
         }
      }
      else
      {
         stack.top()->add_child(NREX_NEW(NRexNodeChar(c[0])));
      }
   }
   if (stack.size() > 1)
   {
      NREX_COMPILE_ERROR("unclosed group '('");
   }
   return true;
}

bool NRex::Match(const NRexChar* str, std::vector<NRexResult>& captures, int offset, int end) const
{
   if (!m_root)
   {
      return false;
   }
   NRexSearch s(str, m_lookaheadDepth);
   if (end >= offset)
   {
      s.end = end;
   }
   else
   {
      s.end = NREX_STRLEN(str);
   }
   for (int i = offset; i <= s.end; ++i)
   {
      for (int c = 0; c <= m_capturing; ++c)
      {
         captures[c].start = 0;
         captures[c].length = 0;
      }
      if (m_root->test(&s, i) >= 0)
      {
         captures = s.m_captures;
         return true;
      }
   }
   return false;
}
