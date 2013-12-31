// Boost.Range library
//
//  Copyright Eric Niebler 2013.
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef RANGES_V3_VIEW_DELIMIT_HPP
#define RANGES_V3_VIEW_DELIMIT_HPP

#include <range/v3/range_fwd.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/utility/iterator_adaptor.hpp>

namespace ranges
{
    inline namespace v3
    {
        namespace detail
        {
            auto delimit_category(std::input_iterator_tag)->std::input_iterator_tag;
            auto delimit_category(std::forward_iterator_tag)->std::forward_iterator_tag;
        }
        template<typename InputIterable, typename Value>
        struct delimit_iterable_view
        {
        private:
            InputIterable rng_;
            Value value_;

            template<bool Const>
            struct basic_iterator
              : ranges::iterator_adaptor<
                    basic_iterator<Const>
                  , range_iterator_t<detail::add_const_if_t<InputIterable, Const>>
                  , use_default
                  , decltype(detail::delimit_category(range_category_t<InputIterable>{}))
                >
            {
            private:
                friend struct delimit_iterable_view;
                friend struct iterator_core_access;

                using iterator_adaptor_ = typename basic_iterator::iterator_adaptor_;
                using base_range = detail::add_const_if_t<InputIterable, Const>;
                using delimit_iterable_view_ = detail::add_const_if_t<delimit_iterable_view, Const>;

                delimit_iterable_view_ *rng_;

                explicit basic_iterator(delimit_iterable_view_ &rng)
                  : iterator_adaptor_{ranges::begin(rng.rng_)}, rng_(&rng)
                {}
                // BUGBUG *If* InputIterable is actually a Range, we need to make sure
                // we don't run off the end.
                bool equal(basic_iterator const &that) const
                {
                    return (nullptr == rng_)
                        ? (nullptr == that.rng_ || *that.base() == that.rng_->value_)
                        : (nullptr != that.rng_ || *this->base() == rng_->value_);
                }
            public:
                basic_iterator()
                  : iterator_adaptor_{}, rng_{}
                {}
                // For iterator -> const_iterator conversion
                template<bool OtherConst, typename std::enable_if<!OtherConst, int>::type = 0>
                basic_iterator(basic_iterator<OtherConst> that)
                  : iterator_adaptor_{std::move(that).base_reference()}, rng_(that.rng_)
                {}
            };
        public:
            using iterator = basic_iterator<false>;
            using const_iterator = basic_iterator<false>;

            delimit_iterable_view(InputIterable && rng, Value value)
              : rng_(std::forward<InputIterable>(rng)), value_(std::move(value))
            {}

            iterator begin()
            {
                return iterator{*this};
            }
            iterator end()
            {
                return {};
            }
            const_iterator begin() const
            {
                return const_iterator{*this};
            }
            const_iterator end() const
            {
                return {};
            }
            bool operator!() const
            {
                return begin() == end();
            }
            explicit operator bool() const
            {
                return begin() != end();
            }
            InputIterable & base()
            {
                return rng_;
            }
            InputIterable const & base() const
            {
                return rng_;
            }
        };

        namespace view
        {
            struct delimiter : bindable<delimiter>
            {
                template<typename InputIterable, typename Value>
                static delimit_iterable_view<InputIterable, Value>
                invoke(delimiter, InputIterable && rng, Value value)
                {
                    return {std::forward<InputIterable>(rng), std::move(value)};
                }

                template<typename Value>
                static auto
                invoke(delimiter delimit, Value value)
                    -> decltype(delimit.move_bind(std::placeholders::_1, std::move(value)))
                {
                    return delimit.move_bind(std::placeholders::_1, std::move(value));
                }
            };

            RANGES_CONSTEXPR delimiter delimit{};
        }
    }
}

#endif
