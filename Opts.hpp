#ifndef OPTS_HPP_INCLUDED
#define OPTS_HPP_INCLUDED

#include <map>
#include <vector>
#include <string>
typedef std::map<std::string, std::string> Opts;

Opts getOpts(int argc, char** argv)
{
    std::vector<std::string> args;
    for(int a=1; a<argc; ++a)
    {
        args.push_back(argv[a]);
    }

    Opts opts;
    for(int a=0; a<args.size(); ++a)
    {
        if(args[a].size()>0 && args[a].substr(0,1)=="-")
        {
            //check if following option is also a -- or - (if it is, this suggests that this option is a boolean)
            if(a==args.size()-1)
            {
                opts[args[a]]="true";
            }
            else if(args[a+1].substr(0,1)=="-")
            {
                opts[args[a]]="true";
            }
            else
            {
                opts[args[a]]=args[a+1];
            }
        }
    }


    return opts;
}


#endif // OPTS_HPP_INCLUDED
