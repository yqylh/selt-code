#include <string>
#include <charconv>
#include <array>
#include <cstdint>
#include <cassert>

// ASCII SOH
constexpr char SOH = '\x01';

/* 将 tag=value<SOH> 追加到 msg 末尾，返回写入字节数 */
template<typename T>
inline void add(std::string& msg, int tag, const T& value) {
    char buf[32];                         // 足够放大多数 int/double
        auto p   = std::to_chars(buf, buf + sizeof(buf), tag);
            *p.ptr++ = '=';
                if constexpr (std::is_same_v<T, double>)
                        p = std::to_chars(p.ptr, buf + sizeof(buf), value, 
                                                  std::chars_format::general, 15);
                                                      else
                                                              p = std::to_chars(p.ptr, buf + sizeof(buf), value);
                                                                  *p.ptr++ = SOH;
                                                                      msg.append(buf, p.ptr);               // 仅一次复制
                                                                      }

                                                                      // 字符串版本
                                                                      inline void add(std::string& msg, int tag, std::string_view s) {
                                                                          char tagbuf[8];
                                                                              auto p = std::to_chars(tagbuf, tagbuf + sizeof(tagbuf), tag);
                                                                                  *p.ptr++ = '=';
                                                                                      msg.append(tagbuf, p.ptr);
                                                                                          msg.append(s);
                                                                                              msg.push_back(SOH);
                                                                                              }

                                                                                              /* 计算 checksum */
                                                                                              inline int checksum(const std::string& msg) {
                                                                                                  uint32_t s = 0;
                                                                                                      for (unsigned char c: msg) s += c;
                                                                                                          return s & 0xFFu;
                                                                                                          }

                                                                                                          std::string build_fix42_fast(/*同前面的字段…*/) {
                                                                                                              std::string msg;
                                                                                                                  msg.reserve(256);         // 预估大小一次到位

                                                                                                                      //-------------------------------- Header
                                                                                                                          add(msg, 8,  "FIX.4.2");
                                                                                                                              size_t bodyLenPos = msg.size();       // 记录 9= 位置
                                                                                                                                  add(msg, 9,  0);                      // 先占位
                                                                                                                                      add(msg, 35, "D");
                                                                                                                                          add(msg, 49, "BROKER12");
                                                                                                                                              add(msg, 56, "EXCHANGE");
                                                                                                                                                  add(msg, 115,"CLIENTA");
                                                                                                                                                      add(msg, 34, 15);
                                                                                                                                                          add(msg, 52, "20250714-11:20:45.123");

                                                                                                                                                              //-------------------------------- Body
                                                                                                                                                                  add(msg, 11, "ORDER-20250714-001");
                                                                                                                                                                      add(msg, 21, 1);
                                                                                                                                                                          add(msg, 55, "AAPL");
                                                                                                                                                                              add(msg, 54, 1);
                                                                                                                                                                                  add(msg, 60, "20250714-11:20:45.123");
                                                                                                                                                                                      add(msg, 40, 2);
                                                                                                                                                                                          add(msg, 109,"C789");
                                                                                                                                                                                              add(msg, 38, 100);
                                                                                                                                                                                                  add(msg, 44, 215.5);
                                                                                                                                                                                                      add(msg, 207,"XNAS");
                                                                                                                                                                                                          add(msg, 1,  "ACC-445566");

                                                                                                                                                                                                              //-------------------------------- 替换 BodyLength
                                                                                                                                                                                                                  size_t afterBodyLen = msg.size();          // 当前位置
                                                                                                                                                                                                                      int bodyLen = static_cast<int>(afterBodyLen - bodyLenPos - 2 /*9=*/ - 1 /*SOH*/);
                                                                                                                                                                                                                          char blBuf[16];
                                                                                                                                                                                                                              auto blEnd = std::to_chars(blBuf, blBuf + sizeof(blBuf), bodyLen);
                                                                                                                                                                                                                                  // 覆盖 9=0<SOH> 中的 “0”
                                                                                                                                                                                                                                      msg.replace(bodyLenPos + 2, blEnd.ptr - blBuf, blBuf, blEnd.ptr - blBuf);

                                                                                                                                                                                                                                          //-------------------------------- Trailer
                                                                                                                                                                                                                                              int csum = checksum(msg);
                                                                                                                                                                                                                                                  add(msg, 10, csum);                     // add() 会自动补足三位
                                                                                                                                                                                                                                                      // 覆盖成三位宽度
                                                                                                                                                                                                                                                          msg.replace(msg.size() - 5, 3,
                                                                                                                                                                                                                                                                          [&]{
                                                                                                                                                                                                                                                                                              char c[4];
                                                                                                                                                                                                                                                                                                                  auto r = std::to_chars(c, c + 4, csum);
                                                                                                                                                                                                                                                                                                                                      if (r.ptr - c == 1) return std::string("00") + c[0];
                                                                                                                                                                                                                                                                                                                                                          if (r.ptr - c == 2) return std::string("0")  + std::string(c,2);
                                                                                                                                                                                                                                                                                                                                                                              return std::string(c,3);
                                                                                                                                                                                                                                                                                                                                                                                              }());

                                                                                                                                                                                                                                                                                                                                                                                                  return msg;
                                                                                                                                                                                                                                                                                                                                                                                                  }