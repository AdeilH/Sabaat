namespace {

    template<typename T>
    concept addable = requires(T a, T b)
    {
        a + b;
    };

    template<addable T>
    class Channel {
        std::mutex m_mutex{};
        std::condition_variable m_not_full{};
        std::condition_variable m_not_empty{};
        std::deque<T> m_queue{};
        std::size_t m_capacity{};
        bool m_done{};

    public:
        explicit Channel(std::size_t capacity): m_capacity(capacity){}
        Channel(const Channel&) = delete;
        Channel& operator=(const Channel&) = delete;
        Channel& operator=(Channel&&) = delete;
        Channel (Channel&&) = delete;

        bool send(T a) {
            std::unique_lock lk{m_mutex};
            m_not_full.wait(lk, [this]() {
               return m_done || m_queue.size() < m_capacity;
           });

            if (m_done) {
                return false;
            }
            m_queue.push_back(std::move(a));
            m_not_empty.notify_one();
            return true;
        }

        bool operator<<(T value) {
            return send(std::move(value));
        }

        std::optional<T> receive() {
            std::unique_lock lk{m_mutex};

            m_not_empty.wait(lk, [this]() {
                return m_done || !m_queue.empty();
            });

            if (m_queue.empty()) {
                return std::nullopt;
            }

            T value = std::move(m_queue.front());
            m_queue.pop_front();
            m_not_full.notify_one();
            return value;
        }

        bool operator>>(T& out) {
            auto val = receive();
            if (val.has_value()) {
                out = std::move(*val);
                return true;
            }
            return false;
        }

        // Close the channel (close(ch))
        void close() {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_done) {
                    return;
                }
                m_done = true;
            }
            m_not_full.notify_all();
            m_not_empty.notify_all();
        }

        [[nodiscard]] bool is_closed() const {
            std::lock_guard lock(m_mutex);
            return m_done;
        }

        [[nodiscard]] std::size_t size() const {
            std::lock_guard lock(m_mutex);
            return m_queue.size();
        }

        ~Channel() {
            close();
        }
    };


}