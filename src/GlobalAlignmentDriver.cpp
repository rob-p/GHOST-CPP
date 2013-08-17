#include <thread>
#include <unordered_map>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "g2logworker.h"
#include "g2log.h"

#include "SignatureMap.hpp"
#include "GraphUtils.hpp"

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
using std::unordered_map;
// using std::unordered_set;
// using std::set;
using boost::filesystem::path;
using boost::filesystem::exists;
using boost::filesystem::is_directory;
using boost::filesystem::create_directories;
namespace po = boost::program_options;

struct AlignOpts {
	boost::optional<uint32_t> numProc;
	boost::optional<uint32_t> localSearch;
	boost::optional<double> alpha;
	boost::optional<double> beta;
	boost::optional<double> unconstrained;
	boost::optional<string> g1;
	boost::optional<string> g2;
	boost::optional<string> s1;
	boost::optional<string> s2;
	boost::optional<string> blastScores;
	boost::optional<string> output;
	boost::optional<string> config;
};

void checkOptions(const AlignOpts& opts) {
	if (!opts.g1) { LOG(FATAL) << "The --g1 option must be provided\n"; }
	if (!opts.g2) { LOG(FATAL) << "The --g2 option must be provided\n"; }
	if (!opts.s1) { LOG(FATAL) << "The --s1 option must be provided\n"; }
	if (!opts.s1) { LOG(FATAL) << "The --s2 option must be provided\n"; }
}

void printOptions(const AlignOpts& opts) {
	std::cerr << "Alignment Options\n";
	std::cerr << "=================\n";

	std::cerr << "numProc = ";
	if (opts.numProc) { std::cerr << *opts.numProc; } else { std::cerr << "undefined"; }
	std::cerr << "\n";

	std::cerr << "localSearch = ";
	if (opts.localSearch) { std::cerr << *opts.localSearch; } else { std::cerr << "undefined"; }
	std::cerr << "\n";

	std::cerr << "alpha = ";
	if (opts.alpha) { std::cerr << *opts.alpha; } else { std::cerr << "undefined"; }
	std::cerr << "\n";

	std::cerr << "beta = ";
	if (opts.beta) { std::cerr << *opts.beta; } else { std::cerr << "undefined"; }
	std::cerr << "\n";

	std::cerr << "unconstrained = ";
	if (opts.unconstrained) { std::cerr << *opts.unconstrained; } else { std::cerr << "undefined"; }
	std::cerr << "\n";

	std::cerr << "g1 = ";
	if (opts.g1) { std::cerr << *opts.g1; } else { std::cerr << "undefined"; }
	std::cerr << "\n";

	std::cerr << "g2 = ";
	if (opts.g2) { std::cerr << *opts.g2; } else { std::cerr << "undefined"; }
	std::cerr << "\n";

	std::cerr << "s1 = ";
	if (opts.s1) { std::cerr << *opts.s1; } else { std::cerr << "undefined"; }
	std::cerr << "\n";

	std::cerr << "s2 = ";
	if (opts.s2) { std::cerr << *opts.s2; } else { std::cerr << "undefined"; }
	std::cerr << "\n";
	
	std::cerr << "blastScores = ";
	if (opts.blastScores) { std::cerr << *opts.blastScores; } else { std::cerr << "undefined"; }
	std::cerr << "\n";
	
	std::cerr << "output = ";
	if (opts.output) { std::cerr << *opts.output; } else { std::cerr << "undefined"; }
	std::cerr << "\n";
}

void parseConfig(AlignOpts& opts, const string& configFile) {
	boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(configFile, pt);

    opts.numProc = pt.get_optional<uint32_t>("main.numProc");
    opts.localSearch = pt.get_optional<uint32_t>("main.localSearch");
	opts.alpha = pt.get_optional<double>("main.alpha");
	opts.beta = pt.get_optional<double>("main.beta");
	opts.unconstrained = pt.get_optional<double>("main.unconstrained");
    opts.g1 = pt.get_optional<string>("main.g1");
    opts.g2 = pt.get_optional<string>("main.g2");
    opts.s1 = pt.get_optional<string>("main.s1");
    opts.s2 = pt.get_optional<string>("main.s2");
    opts.blastScores = pt.get_optional<string>("main.blastScores");
    opts.output = pt.get_optional<string>("main.output");

}

void parseOptions(AlignOpts& opts, const po::variables_map& vm) {

	if (vm.count("numProcessors")) {
		opts.numProc = vm["numProcessors"].as<uint32_t>();
	} else if (!opts.numProc) {
		opts.numProc = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 2);
	}

    if (vm.count("localSearch") or !opts.localSearch) {
    	opts.localSearch = vm["localSearch"].as<uint32_t>();
    }

    if (vm.count("alpha") or !opts.alpha) {
    	opts.alpha = vm["alpha"].as<double>();
    }

    if (vm.count("beta") or !opts.beta) {
    	opts.beta = vm["beta"].as<double>();
    }

    if (vm.count("unconstrained") or !opts.unconstrained) {
    	opts.unconstrained = vm["unconstrained"].as<double>();
    }

    if (vm.count("g1") or !opts.g1) {
    	if (!vm.count("g1")) { LOG(FATAL) << "The --g1 option must be provided\n"; }
    	opts.g1 = vm["g1"].as<string>();
    } 

    if (vm.count("g2") or !opts.g2) {
    	if (!vm.count("g2")) { LOG(FATAL) << "The --g2 option must be provided\n"; }
    	opts.g2 = vm["g2"].as<string>();
    } 

    if (vm.count("s1") or !opts.s1) {
    	if (!vm.count("s1")) { LOG(FATAL) << "The --s1 option must be provided\n"; }
    	opts.s1 = vm["s1"].as<string>();
    } 

    if (vm.count("s2") or !opts.s2) {
    	if (!vm.count("s2")) { LOG(FATAL) << "The --s2 option must be provided\n"; }
    	opts.s2 = vm["s2"].as<string>();
    } 

    if (vm.count("blastScores")) {
    	opts.blastScores = vm["blastScores"].as<string>();
    }

    if (vm.count("output")) {
    	opts.output = vm["output"].as<string>();
    }

}

int globalAlignmentDriver(int argc, char* argv[], const po::variables_map& vm) {

	auto ghostLogo = R"(
   ______ __  __ ____  _____ ______
  / ____// / / // __ \/ ___//_  __/
 / / __ / /_/ // / / /\__ \  / /
/ /_/ // __  // /_/ /___/ / / /
\____//_/ /_/ \____//____/ /_/
  )";

	std::cerr << ghostLogo << "\n";

	/**
	 * Create the structure to hold our program options
	 */
	AlignOpts opts;

	// First read options from the config file if it exists
	if (vm.count("config")) {
		parseConfig(opts, vm["config"].as<string>());
	}
	// Read options from the command line and apply defaults
	parseOptions(opts, vm);
	printOptions(opts);

	/** Read the graphs **/
	auto G = GraphUtils::readGraphFromGEXF(*opts.g1);
	auto H = GraphUtils::readGraphFromGEXF(*opts.g2);

	/** Read the signatures **/
    SignatureMap sigMapG(*opts.s1, *G);
	SignatureMap sigMapH(*opts.s2, *H);

	return 0;
}