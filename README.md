# C++ META API, COMMAND LINE ARGUMENT API

	The original intent was to build API for specifying command line arguments. 
The idea is to have user data structure which will be populated 
with a parsed command line and passed to a processing function. The data structure controls types and names of parameters, as well
as if parameters are named or positional. All positional parameters are treated as required and should be defined first.
It appeared that such approach requires somekind of metadata support which presents value by itself. Unfortuanly the metadata 
API is not extendible and tailored for the original idea, but the general approach can be easily reused.
There is some extra complexity due to metadata being constexpr which required building string classes, but it is optional.
not critical.

The simplest way to read it is to look at main implementaion:

## Usage

```
User data structure defined with macroses which provide metadata under the hood:

struct UserCmd : public BaseCmd{

   typedef UserCmd self;

   FLDN(IntFld,sz,false)   // we define required params first
   FLDN(FlgFld,eval,true)  // optional params are all named ( param #3 is true) and require -- before name (ex. --eval on for boo)
   FLDN(StrFld,sd,true)

   static constexpr BaseCmd::pvec<3> init()
   {
     return { &self::szF, &self::evalF, &self::sdF };
   }
 };

User fucntion which is called with populated data structure: 

 void cmdF(UserCmd& cmd)
 {
     std::cout
       << " Execution:"
       << " sz="  << cmd.sz.v
       << " ev="  << cmd.eval.v
       << " sd="  << cmd.sd.v
       << '\n'  << std::endl;
 }

 int main(int argc, const char * argv[]) {

     AdminMgr adminMgr;

     adminMgr.reg<UserCmd>("tail", cmdF); // register command

     adminMgr.run(argv[1]); // passing as a whole string instead of relying on shell tokenizer, in general not needed


Command line:

$ ./bin/cmd2 "tail 122 --eval on --sd ststr"

Execution: sz=122 ev=1 sd=ststr

```

## Build

Build Option 1 under src:

	make

