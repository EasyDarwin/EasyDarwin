/* 
 * File:   libEasyHttp.cpp
 * Author: wellsen
 * 
 * Created on 2015年10月20日, 下午9:46
 */

#include "libEasyHttp.h"
#include <avhttp.hpp>

void EasyHttp_Get(const char* url)
{
    try
    {
        boost::asio::io_service io;
        avhttp::http_stream h(io);
        boost::system::error_code ec;

        // 打开url.
        h.open(url, ec);
      
        if (ec)
        { // 打开失败处理...
            std::cout << "EasyHttp_Get Error: " << ec.message() << std::endl;
            return;
        }

        boost::array<char, 1024> buf;

        // 循环读取数据.
        while (!ec)
        {
            std::size_t bytes_transferred = h.read_some(boost::asio::buffer(buf), ec);
            // 将下载的数据打印到屏幕.
            std::cout.write(buf.data(), bytes_transferred);
        }

        std::cout.flush();
        h.close(ec); // 关闭.
        //io.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "EasyHttp_Get Error:" << e.what() << std::endl;
    }
}

#ifdef _DEBUG

int main(int argc, char* argv[])
{
    EasyHttp_Get("http://111.1.6.132:11000/AppCamera/CheckDevStateRestaurant?count=10000&isBuildIn=1");
}

#endif

