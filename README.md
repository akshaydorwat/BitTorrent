## Bit Torrent Client

* Akshay Dorwat					adorwat@indiana.edu
* Rohit Khapare					rkhapare@indiana.edu

### Requirement

   This application uses some of the c++ 11 features like std::function, std::bind, std::thread and std::mutex. 
   These features are only supported by the g++ 4.8.2 compiler. 		   	 	      	   		
										
### Features 

* Thread safe Logger which supports multiple stream handles.
* Reactor pattern is implemented using the POLL system call.
* NON - BLOCKING sockets are used to impove performace of the application.
* Thread pool design pattern is used to improve responsiveness and performance of application.
* Highly modular and plugable design which can be enhanced futher.
* Supports N Leecher and N Seeder connectivity.
* Decides Seeder or Leecher role automatically by verifying the hash over the file.

### Folder Description
It is changed a bit.
   *   src - It contain all source code files
   *   sample - All sample input files


### File Description

##### bt_client.c   
      Main file where the control loop lives
     
##### bt_setup.c
      Contains setup code, such as parsing arguments
     
##### bt_lib.c      
      Code for core functionality of bt
      
##### bt_setup.h    
      Header file for setup
      
##### bt_lib.h
      Header file for bt_lib
      
##### threadpool.cpp, threadpool.hpp
      It is basic threadpool implmentation. It is copied from CREDIT(1)
      

##### Logger.cpp, Logger.hpp
      This class implements a thread safe logger. 

##### Peer.cpp, Peer.hpp
      This class will implement peer connection and operation on them.

##### Bencode_t.cpp, Bencode_t.h			
      Abstract base class (interface) implemented by every bencode element

##### BencodeInteger_t.cpp, BencodeInteger_t.h	
      Bencode Integer type definition

##### BencodeString_t.cpp, BencodeString_t.h		
      Bencode String type definition

##### BencodeList_t.cpp, BencodeList_t.h		
      Bencode List type definition


##### BencodeDictionary_t.cpp, BencodeDictionary_t.h	
      Bencode Dictionary type definition

##### BencodeDecoder.cpp, BencodeDecoder.h		
      Bencode Decoder; provides static functions for decoding single/multiple bencodings

##### TorrentFile.cpp, TorrentFile.hpp		
      Type definition for each file listed in a .torrent file

##### Torrent.cpp, Torrent.hpp			
      Type definition of a Torrent file
      

##### Reactor.cpp, Reactor.hpp
      This class has network core functionality. It has 2 main functionality
      1. Accept connection on server socket
      2. Read data from the client sockets connected to server
      3. It uses poll method to do non - blocking accept and read

##### TorrenCtx.cpp TorrentCtx.hpp 			 
      1. Stores Context of the torrent application. 
      2. Decides role of the client i.e Seeder or Leecher.
      3. Lodes the peers.
      4. spawns connection thread in an attempt connect seeder in case of client is playing leecher role.
      5. Calculates the info hash over info dictionary.
      6. start Request making, Request message processing and Piece Processing thread pools depending 
        upon the role client is playing.

##### Peer.cpp Peer.hpp
      1. Stores all information about peer and its state. 
      2. Has core bit torrent protocol implementation. 
      3. Supports HAVE, BITFIELD, INTRESTED, UNCHOCKED, PIECE, REQUEST messages. 
      4. Can encode above protocol message and connection handler to write them into socket
      5. Can decode above protocol message and ask appropriate handler to handle message.

##### ConnectionHandler.cpp, ConnectionHandler.hpp
      1. Handles socket operations on behalf of Peers. 
      2. Buffers incoming messages until full message length is recieved
      3. Indentify Handshake, Live and Protocol messages. 
      4. Verify the handshake on behalf of the peer and sever the connection in case 
      5. invalid peer tried to connect. 
      6. Write message into socket on behalf of the peer. 

##### PieceRequestor.cpp, PieceRequestor.hpp		
      Reads the state of available pieces and issues requests for blocks of data to connected Seeder(s)
      1. Verify the requested parameters like index, begin and length.
      2. Check the availability of the requested data.
      3. Copy data from the Piece Objects.
      4. Send the the data, index and begin to peer to encode Piece message.

##### FileHandler.cpp, FileHandler.hpp		
      Provides the interface to perform read and write operations pertaining to a piece of a torrent (1 TorrentCtx has only 1 FileHandler)


##### Piece.cpp, Piece.hpp				
      1. Maintains the state and/or data of a piece 
      2. Provides interface to perform operations on piece

##### PieceProcessor.cpp, PieceProcessor.hpp		
      1. Processes incoming Piece message to extract block of data

##### RequestProcessor.cpp, RequestProcessor.hpp	
      Processes incoming block requests; verifies correctness and services block of data to requesting Leecher(s)

### Compile

      $ make clean && make

### How to Run

      The generic way to execute the program is as follows ...
      ./bt_client -v -b <local interface> -p 10.0.0.88:6767 -p 10.0.0.2:6767 -s . -l LOG.log  sample/download.mp3.torrent

      For convenience, we have created a list of make targets to test communication between 3 seeders and 3 leechers.
      To use this facility, the following commands need to be issued on 6 different terminal windows RESPECTIVELY 
      on a single host in the order listed below.
      1st on Terminal#1 execute: 	$ make run-s1
      2nd on Terminal#2 execute: 	$ make run-s2
      3rd on Terminal#3 execute: 	$ make run-s3
      4th on Terminal#4 execute:	$ make run-c1
      5th on Terminal#5 execute:	$ make run-c2
      6th on Terminal#6 execute:	$ make run-c3

      If done correctly, 
      Terminal#1 must display the running logs for server#1 (SEEDER) running on 127.0.0.1:6667.
      Terminal#2 must display the running logs for server#2 (SEEDER) running on 127.0.0.1:6668.
      Terminal#3 must display the running logs for server#3 (SEEDER) running on 127.0.0.1:6669.
      Terminal#1 must display the running logs for client#1 (LEECHER) running on 127.0.0.1:6670.
      Terminal#2 must display the running logs for client#2 (LEECHER) running on 127.0.0.1:6671.
      Terminal#3 must display the running logs for client#3 (LEECHER) running on 127.0.0.1:6672.
      
      The folder logs-3seed-3leech/ contains logs produced in one of the previous runs by these 6 processes.
      The seeders and leechers read the sample/download.mp3.torrent (required); 
      seeders read from sample/download.mp3 (required); 
      leechers write to dl-c1/download.mp3, dl-c2/download.mp3, and dl-c3/download.mp3 respectively.
      
      At any point in time, the program may be halted safely by typing Q or q followed by the ENTER keypress.

### Functionality

        The bt_args object contains the arguments provided while invoking the program.
        We extract the torrent file's name (ending with .torrent) from this object and 
        decode the file to obtain the torrent's metadata as a Torrent_t object.
        
        This process takes place as follows.
        The Torrent_t::decode() function opens the .torrent file in binary mode and
        extracts its contents as a std::string. This string is then parsed using 
        BencodeDecoder::decode() function that reads the first character of the string 
        to decide what the outermost bencode element is to call its specific decode function
        i.e. BencodeInteger_t::decode(), BencodeString::decode(), BencodeList::decode() or
        BencodeDictionary::decode(). The last 2 decode() functions (that of List and Dictionary)
        make recursive calls to BencodeDecoder::decode() that returns the abstract base class
        object pointer i.e. Bencode_t*.
        
        A Torrent_t::decode() function expects 1 and only 1 torrent metadata in 
        a single .torrent file. A Torrent_t object may contain multiple TorrentFile_t object(s).
        The extracted torrent metadata is logger using the custom Logger included in
        this submission.
        
        Logger functionality is implementated to handle logging requirement of the project. 
        You can resiter multiple stream with log format for logging. This implmentation is 
        also thread safe.
        
        Non-blocking Server and client functionality is implemented using poll method. 
        It also implemets a singleton and reactor design pattern.
        
        A TorrentCtx is the main Handler for a torrent. It initializes (and encapsulates) 
        all the other objects that operate on the torrent as described in the following.
        
        A single FileHandler instance is shared by all Piece objects for synchronized 
        read/write operations from/to persistent storage. The Piece objects maintain 
        the logical subdivision into blocks (which means there are no Block objects)
        in the form of vectors of state information pertaining to blocks comprising them.
        At the start of the program, the FileHandler loads the status of each piece
        mentioned in the torrent file by looking for the specified files on disk.
        
        When a RequestProcessor demands a Block for servicing to a client/leecher, the
        FileHandler fetches the ENTIRE Piece into memory. We do this on the assumption
        that the leecher requesting this block will demand more than just one block in 
        the near future. This reduces the number of fetch operations and improves Piece service
        performance when compared to fetching each block for each request.
        
        Similarly, the Piece object buffers incoming blocks into its member string
        until a prespecified number of contiguous blocks are received. These contiguous
        blocks are written to the disk in one operation. This reduces the fragmentation
        of write operations and the overhead of mutex locking and unlocking associated
        with each disk I/O. However, once a piece is received in its entirety, this
        delayed write policy is dropped in favor of safeguarding the piece for future
        invocation of the torrent (remember that once a piece is found to be available
        in the disk it will not be requested for).
        
        A PieceRequestor object in a Leecher reads the status of all pieces within a torrent
        and decides (currently pseudo-randomly with preference to already processing piece) 
        as to which piece must be requested and to which Peer the request must be sent to.
        After generating and sending a preset number of MAX_REQUESTS, it waits for the 
        reception of a Piece for a preset REQUEST_TIMEOUT_MILLIS before repeating its operations.
        
        A Piece processor assigns a new piece to it preset threadpool and informs the
        PieceRequestor about the reception so that the PieceRequestor may get unblocked
        for requesting additional piece-blocks if needed.

### Credits

      1. Linux Manual pages
      2. https://github.com/progschj/ThreadPool
      3. https://wiki.theory.org/BitTorrentSpecification
      4. Jim Kurose, Keith Ross, Computer Networking: A Top-Down Approach (6th edition) pg.156-168
      5. Michael Donahoo, Kenneth Calvert, TCP/IP Sockets in C: A Practical Guide for Programmers (2nd Edition) pg.11-32



