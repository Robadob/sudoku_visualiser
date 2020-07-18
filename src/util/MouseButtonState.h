#ifndef SRC_UTIL_MOUSEBUTTONSTATE_H_
#define SRC_UTIL_MOUSEBUTTONSTATE_H_

struct MouseButtonState {
    int left:1;
    int middle:1;
    int right:1;
    int button4:1;
    int button5:1;
};

#endif  // SRC_UTIL_MOUSEBUTTONSTATE_H_