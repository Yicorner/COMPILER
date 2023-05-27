#ifndef DEBUG_H_
#define DEBUG_H_
#include<bits/stdc++.h>
using namespace std;

#define out(args...) { cout << "Line " << __LINE__ << ": [" << #args << "] = ["; debug(args); cout << "]\n"; }
template<typename T>
ostream &operator << (ostream &os, vector<T> &a);
template<typename T>
ostream &operator << (ostream &os, set<T> &a);
template<typename T>
ostream &operator << (ostream &os, multiset<T> &a);
template<typename A, typename B>
ostream &operator << (ostream &os, map<A, B> &a);
template<typename A, typename B>
ostream &operator << (ostream &os, pair<A, B> &a) ;
template<typename T>
ostream &operator << (ostream &os, vector<T> &a) {
    os << "[";
    int f = 0;
    for (auto &x : a) {
        os << (f++ ? ", " : "") << x;
    }
    os << "]";
    return os;
}

template<typename T>
ostream &operator << (ostream &os, set<T> &a) {
    os << "{";
    int f = 0;
    for (auto &x : a) {
        os << (f++ ? ", " : "") << x;
    }
    os << "}";
    return os;
}

template<typename T>
ostream &operator << (ostream &os, multiset<T> &a) {
    os << "{";
    int f = 0;
    for (auto &x : a) {
        os << (f++ ? ", " : "") << x;
    }
    os << "}";
    return os;
}

template<typename A, typename B>
ostream &operator << (ostream &os, map<A, B> &a) {
    os << "{";
    int f = 0;
    for (auto &x : a) {
        os << (f++ ? ", " : "") << x;
    }
    os << "}";
    return os;
}

template<typename A, typename B>
ostream &operator << (ostream &os, pair<A, B> &a) {
    os << "(" << a.first << ", " << a.second << ")";
    return os;
}

template<typename A, typename B, typename C>
ostream &operator << (ostream &os, tuple<A, B, C> &a) {
    os << "(" << get<0>(a) << ", " << get<1>(a) << ", " << get<2>(a) << ")";
    return os;
}

template<typename A, typename B, typename C, typename D>
ostream &operator << (ostream &os, tuple<A, B, C, D> &a) {
    os << "(" << get<0>(a) << ", " << get<1>(a) << ", " << get<2>(a) << ", " << get<3>(a) << ")";
    return os;
}
template<typename T> void debug(T a) {
    cout << a;
}

template<typename T, typename...args> void debug(T a, args...b) {
    cout << a << ", ";
    debug(b...);
}
#endif