/**
 * \file tools.cpp
 * \brief TODO.
 * \author Alexis Devillard
 * \version 1.0
 * \date 08 may 2019
 */

#include "tools.h"
/**
 * @brief error Display the passed string thne exit the program.
 * @param str String to display.
 */
void error(std::string str)
{
  std::cout << str << std::endl;
  exit(0);
}

/**
 * @brief usage Display the usage, then exit the program.
 * @param optf List of option flags
 * @param optf List of option labels
 * @param optf List of option values
 */
void usage(std::vector<std::string>& optf,  std::vector<std::string>& optl, std::vector<std::string>& optv)
{
  std::cout << "Usage: ./lslpub_OTB [OPTION ...]" << std::endl;
  std::cout << "Options: " << std::endl;
  for(int i = 0; i< optf.size(); i++)
    std::cout << "         " << optf[i] << "\t" << optl[i] <<" (ex: " << optv[i] << " )"<< std::endl;
  exit(0);
}

/**
 * @brief get_arg Search for the potential argument in the argument passed to the program.
 * @param argc Argument counter
 * @param argv Argument array
 * @param optf List of option flags
 * @param optf List of option labels
 * @param optf List of option values
 */
void get_arg(int argc, char ** argv, std::vector<std::string>& optf,  std::vector<std::string>& optl, std::vector<std::string>& optv)
{
  int i =1;
  std::string help_flag = "-h";
  while(i < argc)
    {
      if(help_flag.compare(argv[i])==0)
	usage(optf,optl,optv);
      int j = 0;
      while(optf[j].compare(argv[i]) != 0)
	{
	  j++;
	  if(j>= optf.size())
	    usage(optf,optl,optv);
	}
      if(i+1 >= argc || argv[i+1][0] == '-')
	usage(optf,optl,optv);
      optv[j] = argv[i+1];
      i+=2; 
    }
  for(int i = 0; i< optl.size(); i++)
    std::cout << optl[i] <<" : " << optv[i] << std::endl;
}
