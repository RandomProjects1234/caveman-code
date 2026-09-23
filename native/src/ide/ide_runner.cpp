#include "ide_runner.hpp"

#include <chrono>

#include "errors.hpp"

namespace og {

RunManager::RunManager(OutputFn on_output, DoneFn on_done)
    : on_output_(std::move(on_output)), on_done_(std::move(on_done)) {}

bool RunManager::start(const std::string& source, const std::string& filename,
                       cmc::DrawSurface* draw) {
    if (running_) return false;
    running_ = true;
    stopping_ = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        has_pending_input_ = false;
        has_answer_ = false;
        pending_prompt_.clear();
        answer_.clear();
    }

    if (thread_.joinable()) thread_.join();

    thread_ = std::thread([this, source, filename, draw] {
        cmc::Interpreter interpreter;
        interpreter.set_output([this](const std::string& line) {
            on_output_(line, "out");
        });
        interpreter.set_input([this](const std::string& prompt, int line) {
            return ask(prompt, line);
        });
        interpreter.set_draw(draw);
        interpreter.set_should_stop([this] { return stopping_.load(); });
        interpreter.set_sleep([this](double seconds) { sleep_for(seconds); });

        RunOutcome outcome;
        try {
            interpreter.run(source, filename);
        } catch (cmc::CmcStopped&) {
            outcome.kind = RunOutcome::Kind::Stopped;
        } catch (cmc::CmcError& error) {
            outcome.kind = RunOutcome::Kind::Error;
            outcome.message = error.format();
        } catch (std::exception& error) {
            outcome.kind = RunOutcome::Kind::Error;
            outcome.message = std::string("CMC had a bug: ") + error.what();
        }

        running_ = false;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            has_pending_input_ = false;
            has_answer_ = true;
        }
        cv_.notify_all();
        on_done_(outcome);
    });
    return true;
}

void RunManager::stop() {
    if (!running_) return;
    stopping_ = true;
    cv_.notify_all();
}

std::string RunManager::ask(const std::string& prompt, int line) {
    std::unique_lock<std::mutex> lock(mutex_);
    pending_prompt_ = prompt;
    pending_line_ = line;
    has_pending_input_ = true;
    has_answer_ = false;
    lock.unlock();
    cv_.notify_all();

    lock.lock();
    cv_.wait_for(lock, std::chrono::milliseconds(50), [this] {
        return has_answer_ || stopping_.load();
    });
    while (!has_answer_ && !stopping_.load()) {
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        lock.lock();
    }
    if (stopping_.load()) {
        has_pending_input_ = false;
        throw cmc::CmcStopped();
    }
    std::string value = answer_;
    answer_.clear();
    has_answer_ = false;
    return value;
}

void RunManager::sleep_for(double seconds) {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait_for(lock, std::chrono::duration<double>(seconds),
                 [this] { return stopping_.load(); });
}

std::optional<std::pair<std::string, int>> RunManager::pending_input() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!has_pending_input_) return std::nullopt;
    return std::make_pair(pending_prompt_, pending_line_);
}

void RunManager::clear_pending_input() {
    std::lock_guard<std::mutex> lock(mutex_);
    has_pending_input_ = false;
}

void RunManager::answer_input(const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    answer_ = value;
    has_answer_ = true;
    cv_.notify_all();
}

}