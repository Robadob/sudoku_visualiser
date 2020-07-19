#ifndef SRC_UTIL_MOUSEBUTTONSTATE_H_
#define SRC_UTIL_MOUSEBUTTONSTATE_H_

struct MouseButtonState {
    unsigned int left:1;
    unsigned int middle:1;
    unsigned int right:1;
    unsigned int button4:1;
    unsigned int button5:1;
};

#endif  // SRC_UTIL_MOUSEBUTTONSTATE_H_
