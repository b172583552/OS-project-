// userkernel.cc 
//	Initialization and cleanup routines for the version of the
//	Nachos kernel that supports running user programs.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchconsole.h"
#include "userkernel.h"
#include "synchdisk.h"
#include "synchconsole.h"

//----------------------------------------------------------------------
// UserProgKernel::UserProgKernel
// 	Interpret command line arguments in order to determine flags 
//	for the initialization (see also comments in main.cc)  
//----------------------------------------------------------------------

UserProgKernel::UserProgKernel(int argc, char **argv) 
		: ThreadedKernel(argc, argv)
{
    debugUserProg = FALSE;
    consoleIn = NULL;
    consoleOut = NULL;
    for (int i = 0; i < NumPhysPages; i++)
        PhysicalPageUsed[NumPhysPages] = FALSE;
	execfileNum = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
		    debugUserProg = TRUE;
		}
		else if (strcmp(argv[i], "-e") == 0) {
			execfile[++execfileNum]= argv[++i];
		}
		//<TODO>
        // Get execfile & its priority & burst time from argv, then save them.
		else if (strcmp(argv[i], "-epb") == 0) {
            execfile[++execfileNum] = argv[++i];
            threadPriority[execfileNum] = atoi(argv[++i]);
            threadRemainingBurstTime[execfileNum] = atoi(argv[++i]);
	    }
	    //<TODO>
	    else if (strcmp(argv[i], "-u") == 0) {
			cout << "===========The following argument is defined in userkernel.cc" << endl;
			cout << "Partial usage: nachos [-s]\n";
			cout << "Partial usage: nachos [-u]" << endl;
			cout << "Partial usage: nachos [-e] filename" << endl;
		}
		else if (strcmp(argv[i], "-h") == 0) {
			cout << "argument 's' is for debugging. Machine status  will be printed " << endl;
			cout << "argument 'e' is for execting file." << endl;
			cout << "atgument 'u' will print all argument usage." << endl;
			cout << "For example:" << endl;
			cout << "	./nachos -s : Print machine status during the machine is on." << endl;
			cout << "	./nachos -e file1 -e file2 : executing file1 and file2."  << endl;
		}
    }
}

//----------------------------------------------------------------------
// UserProgKernel::Initialize
// 	Initialize Nachos global data structures.
//----------------------------------------------------------------------

void
UserProgKernel::Initialize()
{
    ThreadedKernel::Initialize();	// init multithreading

    machine = new Machine(debugUserProg);
    fileSystem = new FileSystem();


    currentThread = new Thread("main", threadNum++);	
    synchConsoleIn = new SynchConsoleInput(consoleIn);
    synchConsoleOut = new SynchConsoleOutput(consoleOut);  

    currentThread->setStatus(RUNNING);

    interrupt->Enable();
    
	#ifdef FILESYS
	    synchDisk = new SynchDisk("New SynchDisk");
	#endif // FILESYS
}

//----------------------------------------------------------------------
// UserProgKernel::~UserProgKernel
// 	Nachos is halting.  De-allocate global data structures.
//	Automatically calls destructor on base class.
//----------------------------------------------------------------------

UserProgKernel::~UserProgKernel()
{
    delete fileSystem;
    delete machine;
#ifdef FILESYS
    delete synchDisk;
#endif
    delete synchConsoleIn;
    delete synchConsoleOut;
}

//----------------------------------------------------------------------
// UserProgKernel::Run
// 	Run the Nachos kernel.  For now, just run the "halt" program. 
//----------------------------------------------------------------------
void
UserProgKernel::Run()
{

	/*cout << "Total threads number is " << execfileNum << endl;
	for (int n=1;n<=execfileNum;n++)
	{
		t[n] = new Thread(execfile[n]);
		t[n]->space = new AddrSpace();
		t[n]->Fork((VoidFunctionPtr) &ForkExecute, (void *)t[n]);
		cout << "Thread " << execfile[n] << " is executing." << endl;
	}*/
//	Thread *t1 = new Thread(execfile[1]);
//	Thread *t1 = new Thread("../test/test1");
//	Thread *t2 = new Thread("../test/test2");

//    AddrSpace *halt = new AddrSpace();
//	t1->space = new AddrSpace();
//	t2->space = new AddrSpace();

//    halt->Execute("../test/halt");
//	t1->Fork((VoidFunctionPtr) &ForkExecute, (void *)t1);
//	t2->Fork((VoidFunctionPtr) &ForkExecute, (void *)t2);

    // ThreadedKernel::Run();
//	cout << "after ThreadedKernel:Run();" << endl;	// unreachable
}

//----------------------------------------------------------------------
// UserProgKernel::SelfTest
//      Test whether this module is working.
//----------------------------------------------------------------------

void
UserProgKernel::SelfTest() {
/*    char ch;

    ThreadedKernel::SelfTest();

    // test out the console device

    cout << "Testing the console device.\n" 
	<< "Typed characters will be echoed, until q is typed.\n"
    	<< "Note newlines are needed to flush input through UNIX.\n";
    cout.flush();

    SynchConsoleInput *input = new SynchConsoleInput(NULL);
    SynchConsoleOutput *output = new SynchConsoleOutput(NULL);

    do {
    	ch = input->GetChar();
    	output->PutChar(ch);   // echo it!
    } while (ch != 'q');

    cout << "\n";

    // self test for running user programs is to run the halt program above
*/


//	cout << "This is self test message from UserProgKernel\n" ;
}


void
ForkExecute(Thread *t)
{
     cout << "Thread: " << (void *) t << endl;
    //<TODO>
        ASSERT(t != nullptr);

    // Assuming the thread object has a filename property
    char *filename = t->filename;  // Or retrieve the filename by appropriate means

    OpenFile *executable = fileSystem->Open(filename);
    if (executable == nullptr) {
        cout << "Unable to open file: " << filename << endl;
        return;  // Exit if file not found
    }

    AddrSpace *space = new AddrSpace(executable);
    if (!space->IsValid()) {
        cout << "Failed to initialize address space from file: " << filename << endl;
        delete executable;  // Cleanup the executable file object
        delete space;       // Cleanup the created address space
        return;             // Exit if failed to create valid address space
    }

    t->space = space; // Assign the address space to the thread

    // Close the executable file as it is no longer needed
    delete executable;

    // Now, set the initial register values and page table for the process
    space->InitRegisters();
    space->RestoreState();

    // Move the thread to the ready queue and make it ready to run
    machine->Run(); // This function will not return unless there is an error

    // If machine->Run returns, there was an error
    ASSERT(FALSE);
    
    // When Thread t goes to Running state in the first time, its file should be loaded & executed.
    // Hint: This function would not be called until Thread t is on running state.
    //<TODO>
}

int 
UserProgKernel::InitializeOneThread(char* name, int priority, int burst_time)
{
    //<TODO>
    if (threadNum >= MAX_THREADS) {  // Assume MAX_THREADS is defined somewhere
        cerr << "Maximum number of threads reached, cannot create new thread." << endl;
        return -1;
    }

    // Create a new thread; assume constructor sets thread to JUST_CREATED
    t[threadNum] = new Thread(name, priority, burst_time);

    // Set additional properties if needed, e.g., priority, burst time
    t[threadNum]->SetPriority(priority);
    t[threadNum]->SetBurstTime(burst_time);

    // Load the executable file into the address space of the thread
    OpenFile *executable = fileSystem->Open(name);
    if (executable == nullptr) {
        cerr << "Unable to open executable file: " << name << endl;
        delete t[threadNum];
        t[threadNum] = nullptr;
        return -1;
    }

    AddrSpace *space = new AddrSpace(executable);
    if (!space->IsValid()) {
        cerr << "Failed to initialize address space from file: " << name << endl;
        delete executable;
        delete space;
        delete t[threadNum];
        t[threadNum] = nullptr;
        return -1;
    }

    // When each execfile comes to Exec function, Kernel helps to create a thread for it.
    // While creating a new thread, thread should be initialized, and then forked.
    t[threadNum]->space = new AddrSpace();
    delete executable;
    t[threadNum]->Fork((VoidFunctionPtr) &ForkExecute, (void *)t[threadNum]);
    //<TODO>

    threadNum++;
    return threadNum - 1;
}

void 
UserProgKernel::InitializeAllThreads()
{
    for (int i = 1; i <= execfileNum; i++){
        // cout << "execfile[" << i << "]: " << execfile[i] << " start " << endl;
        int a = InitializeOneThread(execfile[i], threadPriority[i], threadRemainingBurstTime[i]);
        // cout << "execfile[" << i << "]: " << execfile[i] << " end "<< endl;
    }
    // After InitializeAllThreads(), let main thread be terminated that we can start to run our thread.
    currentThread->Finish();
    // kernel->machine->Run();
}