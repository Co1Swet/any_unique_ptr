#pragma once

#include <memory>
#include <typeinfo>
#include <utility>
#include <typeindex>

namespace MulNXB {
    namespace detail {
        class any_base {
        public:
            virtual ~any_base() = default;

            virtual const std::type_info& type() const = 0;
            virtual void* data() noexcept = 0;
            virtual const void* data() const noexcept = 0;
            virtual any_base* clone() const = 0;

            any_base() = default;
            any_base(any_base&&) noexcept = default;
            any_base& operator=(any_base&&) noexcept = default;

            any_base(const any_base&) = default;
            any_base& operator=(const any_base&) = default;
        };

        template<typename T>
        class any_derived : public any_base {
            T value_;
        public:
            template<typename... Args>
            any_derived(Args&&... args)
                : value_(std::forward<Args>(args)...) {
            }

            const std::type_info& type() const override {
                return typeid(T);
            }

            void* data() noexcept override {
                return &value_;
            }

            const void* data() const noexcept override {
                return &value_;
            }

            T& get() { return value_; }
            const T& get() const { return value_; }

            any_base* clone() const override {
                return new any_derived<T>(*this);
            }
        };
    } // namespace detail

    class any_unique_ptr {
        detail::any_base* ptr_;

    public:
        any_unique_ptr() noexcept : ptr_(nullptr) {}
        explicit any_unique_ptr(detail::any_base* ptr) noexcept : ptr_(ptr) {}

        any_unique_ptr(any_unique_ptr&& other) noexcept
            : ptr_(other.release()) {
        }

        any_unique_ptr& operator=(any_unique_ptr&& other) noexcept {
            if (this != &other) {
                reset(other.release());
            }
            return *this;
        }

        any_unique_ptr(const any_unique_ptr&) = delete;
        any_unique_ptr& operator=(const any_unique_ptr&) = delete;

        ~any_unique_ptr() {
            reset();
        }

        detail::any_base* release() noexcept {
            detail::any_base* temp = ptr_;
            ptr_ = nullptr;
            return temp;
        }

        void reset(detail::any_base* ptr = nullptr) noexcept {
            if (ptr_ != ptr) {
                delete ptr_;
                ptr_ = ptr;
            }
        }

        void swap(any_unique_ptr& other) noexcept {
            std::swap(ptr_, other.ptr_);
        }

        detail::any_base* get() noexcept { return ptr_; }
        const detail::any_base* get() const noexcept { return ptr_; }

        template<typename T>
        T* get() {
            if (!ptr_) return nullptr;
            if (ptr_->type() != typeid(T)) return nullptr;
            auto* derived = static_cast<detail::any_derived<T>*>(ptr_);
            return &derived->get();
        }

        template<typename T>
        const T* get() const {
            if (!ptr_) return nullptr;
            if (ptr_->type() != typeid(T)) return nullptr;
            auto* derived = static_cast<const detail::any_derived<T>*>(ptr_);
            return &derived->get();
        }

        template<typename T>
        T* get_unsafe() const {
            if (!ptr_) return nullptr;
            auto* derived = reinterpret_cast<detail::any_derived<T>*>(ptr_);
            if (derived) return &derived->get();
            return nullptr;
        }

        // 安全地提取为独占指针：构造一个新的 T（移动内部值），删除 any_base，返回拥有的 T
        template<typename T>
        std::unique_ptr<T> to_unique() {
            if (!ptr_) return nullptr;
            if (ptr_->type() != typeid(T)) return nullptr;

            auto* derived = static_cast<detail::any_derived<T>*>(ptr_);
            // 移动构造一个新的 T，确保生命周期正确
            std::unique_ptr<T> result = std::make_unique<T>(std::move(derived->get()));
            // 删除原有容器并清空指针
            delete ptr_;
            ptr_ = nullptr;
            return result;
        }

        explicit operator bool() const noexcept {
            return ptr_ != nullptr;
        }

        const std::type_info& type() const noexcept {
            return ptr_ ? ptr_->type() : typeid(void);
        }

        any_unique_ptr clone() const {
            if (ptr_) {
                return any_unique_ptr(ptr_->clone());
            }
            return any_unique_ptr();
        }
    };

    template<typename T, typename... Args>
    any_unique_ptr make_any_unique(Args&&... args) {
        return any_unique_ptr(new detail::any_derived<T>(std::forward<Args>(args)...));
    }
}