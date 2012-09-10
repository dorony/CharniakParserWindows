DISCLAIMER/LICENSE/BLA

*****************************
Hello there!
This code is provided "as is" without express or implied warranty. All rights are reserved to Brown University, authors of the original Linux version of this software, and the license remains as explained in the README file.
This code was never used in a production environment, and you are using it at your own risk. So, if your monitor explodes, that's your problem, not ours nor Brown University's. 
*****************************

Compilation:

To compile this code you can open it in Visual Studio 2012 and compile it. You should compile it in Release mode if you don't want it to be extremely slow. It can be compiled as both win32 and x64 executables.

How to run:
from command line ->
Charniak.exe <Path to data folder (e.g. /DATA/EN/)> <Path to input file>

The input file format is as follows:
<s> sentence1 </s>
<s> sentence2 </s>
...


NOTES:

* This is NOT the multithreaded version of the parser, only the single-threaded version was compiled on Windows.

ENJOY