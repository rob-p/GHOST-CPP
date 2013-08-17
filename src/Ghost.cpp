
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "g2logworker.h"
#include "g2log.h"

/** Namespace uses */
using std::string;
using std::ios;
using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::exception;
using std::abort;
// using std::ifstream;
// using std::ofstream;
// using std::map;
// using std::unordered_map;
// using std::unordered_set;
// using std::set;
using boost::filesystem::path;
using boost::filesystem::exists;
using boost::filesystem::is_directory;
using boost::filesystem::create_directories;
namespace po = boost::program_options;
// using boost::adjacency_list;
// using boost::graph_traits;
// using boost::vecS;
// using boost::listS;
// using boost::variant;
// using boost::lexical_cast;


int globalAlignmentDriver(int argc, char* argv[], const po::variables_map& vm);

int main(int argc, char* argv[]) {

    using std::exception;

    // The options to choose the method (parsimony | probabilistic)
    po::options_description cmd("commands");
    cmd.add_options()
    ("command", po::value< string >(), "command to use; one of {gensig, align}");

    // The help option
    po::options_description help("Help Options");
    help.add_options()
    ("help,h", po::value<bool>()->zero_tokens(), "display help for a specific method");

    // The general options are relevant to either method
    po::options_description general("Genearal Options");
    general.add_options()
    ("numProcessors,p", po::value< uint32_t >(), "number of processors to use in parallel");
    ;

    // Those options only relevant to the parsimony method
    po::options_description align("Alignment Options");
    align.add_options()
    ("localSearch,l", po::value< uint32_t >()->default_value(10), "number of iterations of local search to use")
    ("alpha,a", po::value< double >()->default_value(1.0), "alpha() parameter (trade-off between sequence & topology")
    ("beta,b", po::value< double >()->default_value(2.0), "beta parameter (BLAST e-value cutoff)")
    ("unconstrained,r", po::value< double >()->default_value(10), "percentage of local swaps which should be allowed to be unconstrained")
    ("g1,u", po::value< string >(), "first input graph")
    ("g2,v", po::value< string >(), "second input graph")
    ("s1,s", po::value< string >(), "first input signature file")
    ("s2,t", po::value< string >(), "second input signature file")
    ("blast,b", po::value< string >(), "pairwise BLAST scores")
    ("output,o", po::value< string >(), "output alignemnt file")
    ("config,c", po::value< string >(), "configuration file")
    ;

    po::positional_options_description pos;
    pos.add("command", 1);

    // The options description to parse all of the options
    po::options_description all("Allowed options");
    all.add(help).add(cmd).add(general).add(align);

    try {
    	po::variables_map vm;
    	po::store(po::command_line_parser(argc, argv).positional(pos).options(all).run(), vm);

    	if ( vm.count("help") ){
    		std::cout << "GHOST\n";
    		std::cout << all << std::endl;
    		std::exit(1);
    	}
    	/*
    	if ( vm.count("cfg") ) {
    		std::cerr << "have detected configuration file\n";
    		string cfgFile = vm["cfg"].as<string>();
    		std::cerr << "cfgFile : [" << cfgFile << "]\n";
    		po::store(po::parse_config_file<char>(cfgFile.c_str(), programOptions, true), vm);
    	}
    	po::notify(vm);
     	*/

        string logFilePath(".");
        g2LogWorker logger(argv[0], logFilePath);
        g2::initializeLogging(&logger);

        // For any combination of cmd line arguments that won't actually run the program
        // print the appropriate message and exit
        //printHelpUsage(ap, general, prob, parsimony);
        auto command = vm["command"].as<std::string>();
        std::cerr << "command is : " << command << "\n";
        if (command == "align") {
        	globalAlignmentDriver(argc, argv, vm);
        }

        // boost::property_tree::ptree pt;
        // boost::property_tree::ini_parser::read_ini("config.ini", pt);
        // std::cout << pt.get<std::string>("Section1.Value1") << std::endl;
        // std::cout << pt.get<std::string>("Section1.Value2") << std::endl;


    } catch (exception &e) {
        LOG(FATAL) << "Caught Exception: [" << e.what() << "]\n";
        abort();
    }

    return 0;


}