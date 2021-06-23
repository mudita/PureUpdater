#include "helper.hpp"

#include <boost/process/io.hpp>
#include <boost/process/system.hpp>

void unpack(std::string file, std::string dir)
{
    boost::process::ipstream is;
    printf("FROM %s TO %s\n ",file.c_str(), dir.c_str());
    boost::process::system("tar -xvf " + file + " -C " + dir);//, boost::process::std_out > is);
    //WARNING ERROR
    //BOOST_ASSERT(code);
}
