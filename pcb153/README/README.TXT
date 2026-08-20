The modified THREAD module provides precommitted stack, thus
offering a temporary workaround to the thread-stacks-larger-
than-4K problem. This is definitly not a final solution and
users can expect additional materials in the future.

If it is desired that default behavior be enforced, the
global variable COMMITSTACK has been provided. When
COMMITSTACK is set to 0, the stack is not precommitted; when
set to 1 (the default), the stack is precommitted. I have
zero idea why a user might use this "feature" but I implemented
it so that I could easily regress the library to test further
possible solutions. Its there, tho, if you need it.

The module THREAD.OBJ may be used by inserting it directly
into a project file or by replacing the exiting THREAD.OBJ
module in the C2MT.LIB static multithreaded library. This is
done with the following command:

              TLIB [d:]\BCOS2\LIB\C2MT.LIB +-THREAD.OBJ

The source code has also been provided so that those who are
using the dynamic RTL can rebuild it to incorporate the modified
thread module.