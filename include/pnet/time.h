// Copyright 2014

#ifndef PNET_TIME_H_
#define PNET_TIME_H_

#include <ctime>
#include <string>

namespace pnet {

  class Time {

    public:
      Time() : sec_(0), usec_(0) { }

      Time(uint32_t sec, uint32_t usec) : sec_(sec), usec_(usec) { }

      explicit Time(std::string str) {
        char *p;
        sec_ = std::strtoul(str.c_str(), &p, 10);
        usec_ = std::strtoul(str.c_str() + 11, &p, 10);
      }

      explicit Time(const struct timeval &t)
          : sec_(t.tv_sec), usec_(t.tv_usec) { }

      explicit Time(const struct timespec &t)
          : sec_(t.tv_sec), usec_(t.tv_nsec) { }

      void operator=(const struct timeval &t) {
        sec_ = t.tv_sec;
        usec_ = t.tv_usec;
      }

      void operator=(const struct timespec &t) {
        sec_ = t.tv_sec;
        sec_ = t.tv_nsec;
      }

      Time operator-(const Time &t) const {
        int32_t diff_sec = sec_ - t.sec_;
        int32_t diff_usec = usec_ - t.usec_;
        if (diff_usec < 0)
          return Time(--diff_sec, diff_usec + 1e6);
        return Time(diff_sec, diff_usec);
      }

      uint64_t milliseconds() const {
        return (static_cast<uint64_t>(sec_) * 1e6) +
               static_cast<uint64_t>(usec_);
      }

      bool operator<(const Time &t) const {
        return this->milliseconds() < t.milliseconds();
      }

      std::string ToHumanDate() const {
        time_t t = sec_;
        char timestring[256], microseconds[8];
        strftime(timestring, sizeof(timestring), "%F_%T", localtime(&t));
        sprintf(microseconds, ".%06u", usec_);
        return std::string(timestring) + std::string(microseconds);
      }

      std::string to_string() const {
        char buffer[32];
        snprintf(buffer, 32, "%u.%06u", sec_, usec_);
        return std::string(buffer);
      }

      static std::string now() {
        char temp[24];
        time_t now = time(0);
        strftime(temp, sizeof(temp), "%F_%T", localtime(&now));
        return std::string(temp);
      }

    public:
      uint32_t sec_;
      uint32_t usec_;
  };

  class Timer {
    public:
      Timer() { }

      void start() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        t_start_ = ts;
      }

      Time stop() {
        struct timespec te;
        clock_gettime(CLOCK_MONOTONIC, &te);
        return Time(te) - t_start_;
      }

    private:
      Time t_start_;
  };

} //namespace pnet


#endif  // PNET_TIME_H_
