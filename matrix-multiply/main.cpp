/////////////////////////
//  Joshua Hernandez   //
//  905752791          //
//  jh27539@vt.edu     //
//  ECE 3574           //
/////////////////////////

/*A multi-threaded console application to perform [M × N] × [N × P] 
matrix multiplication to produce an [M `× P] matrix using POSIX threads (pthreads).*/

//** OBJECTIVES **//
//Reading the two input files must be done in parallel, using two threads spawned by the main thread.
//Once the multiplication is done, the parent thread (main thread) may write the resultant C matrix in the output file.

#include <QDebug>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <pthread.h>
#include <unistd.h>

#define NUM_INPUTS 2

//Matrix Dimensions
int M, N, P;
int N1, N2;

//Matrices
QList<double> A;
QList<double> B;
QList<double> C;

struct init_thread_data{
	QString fileName;
	int fileNumber;
};

struct multiply_thread_data{
	int tid;
	int row;
	int column;
	double product;
};


//**  Prototypes  **//
void checkInputDimensions();
bool argumentNumber( int number );
bool argsAreValid( char * arg1, char * arg2, char * arg3 );
void initThread(QString file1, QString file2);
void matrixInit(QString matrix, int fileNumber);
void generateMatrix(QStringList list, int fileNumber, int rows, int columns);
void matrixMultiply();
void outputMatrix(QString output);

// worker threads
void *initRunner(void *fileStruct);
void *multiplyRunner(void *multiplyStruct);


int main( int argc, char * argv[] ){

	if( argumentNumber(argc) || argsAreValid( argv[1], argv[2], argv[3] ))
		return 0;

	QString firstFile = argv[1];
	QString secondFile = argv[2];
	QString thirdFile = argv[3];

	initThread(firstFile, secondFile);
	checkInputDimensions();

	matrixMultiply();
	outputMatrix(thirdFile);

	qDebug() << "Silence is golden :)";

	return 0;
}


//**   Multi-Threading   **//

// This function CREATES 2 threads that read each matrix file
void initThread(QString file1, QString file2){

	pthread_t threads[NUM_INPUTS];
	QString files[NUM_INPUTS];
	int i, rc;

	files[0] = file1;
	files[1] = file2;

	struct init_thread_data args[NUM_INPUTS];

	for(int i = 0; i < NUM_INPUTS; i++ ) {		

		args[i].fileName = files[i];
		args[i].fileNumber = i;

		rc = pthread_create(&threads[i], NULL, initRunner, &args[i]);

		//rc is an error code generated by POSIX
		// a 0 is returned on successful pthread_create
      	if (rc) {
         	qDebug() << "Error:unable to create thread," << rc << endl;
         	exit(-1);
      	}
   	}

   	// Keep separate from above FOR LOOP
   	// so each thread does not need to wait for previous thread to finish
   	for(i=0; i < NUM_INPUTS; i++ )
		pthread_join(threads[i], NULL);
}

// This is the thread working
void *initRunner(void *fileStruct) {

	struct init_thread_data *data = (struct init_thread_data*) fileStruct;
	QString file = data->fileName;

	//Reads in the matrix
	matrixInit(file, data->fileNumber);

   	pthread_exit(0);
}


//**  Other Functions  **//

void matrixInit(QString matrix, int fileNumber){
	QStringList list;
	int columns, rows = 0;

    //Set up file containing the Matrix
	QFile file(matrix);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
       	return;

    //Prime Read (checks the matrix columns)
    QString line = file.readLine();
    list.append(line.split(QRegExp("[\r\n\t ]+"), QString::SkipEmptyParts));

    //Find the number of entries per row
    columns = line.split(QRegExp("[\r\n\t ]+"), QString::SkipEmptyParts).size();
    rows++;

    //Read in the rest of the matrix
    while (!file.atEnd()) {
    	QString line = file.readLine();
    	list.append(line.split(QRegExp("[\r\n\t ]+"), QString::SkipEmptyParts));
    	if(line.split(QRegExp("[\r\n\t ]+"), QString::SkipEmptyParts).size() != columns){
    		qDebug() << "Mismatching columns in row " << rows+1 << " of " << matrix;
    		exit (EXIT_FAILURE);
    	}
    	rows++;
	}

	//Update global variables to keep track of matrix dimensions
	generateMatrix(list, fileNumber, rows, columns);
}

void generateMatrix(QStringList list, int fileNumber, int rows, int columns){

	if(fileNumber == 0){
		M = rows;
		N1 = columns;
		foreach(QString num, list)
    		A.append(num.toDouble());
    	//qDebug() << A;
	}
	else if(fileNumber == 1){
		N2 = rows;
		P = columns;
		foreach(QString number, list)
    		B.append(number.toDouble());
    	//qDebug() << B;
	}
}

void matrixMultiply(){
	
	int i, j, rc;
	int count = 0;
	int NUM_THREADS = M*P;

	pthread_t threads[NUM_THREADS];
	struct multiply_thread_data args[NUM_THREADS];

	for(i = 0; i < M; i++) {
	  for(j = 0; j < P; j++) {

	    args[count].row = i;
	    args[count].column= j;	  
	    args[count].tid = count; 
	    args[count].product = 0;    

	    //Create the thread
	    rc = pthread_create(&threads[count], NULL, multiplyRunner, &args[count]);
	    if (rc) {
        	qDebug() << "Error:unable to create thread," << rc << endl;
        	exit(-1);
      	}
      	count++;
	  }
	}

	// Wait for all threads to multiply before continuing
   	for(i=0; i < NUM_THREADS; i++ )
		pthread_join(threads[i], NULL);

	// Store products all workers into final list
	for(i=0; i<NUM_THREADS; i++)
		C.append(args[i].product);
}

void *multiplyRunner(void *multiplyStruct){

	struct multiply_thread_data *data = (struct multiply_thread_data*) multiplyStruct;

   //FINALLY WE MULTIPLY
   for(int i=0; i<N; i++)
      data->product += A[ (data->row*M) + i] * B[ data->column + (i*P) ];

	pthread_exit(0);
}

void outputMatrix(QString output){

	QFile file(output);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    int count = 0;

    QTextStream out(&file);
    for(int i=0; i<M; i++){
    	for(int j=0; j<P; j++){
    		//Tab in between each element
    		out << C[count] << "\t";
    		count++;
    	}
    	//add new line at the end of each row
    	out << "\n";
    }
}

//**  Error Checking   **//

bool argumentNumber( int number )
{
	//Check for correct number of arguments
	if (number != 4){
		qDebug() << "Invalid number of arguments!" << '\n' <<
		"Please use this format: ./matrix-multiply <input-file1> <input-file2> <outfile>";
		return true;
	}
	else
		return false;
}

void checkInputDimensions(){

	if(!(N1==N2)){
		qDebug() << "[ M x N ] = [" << M << "x" << N1 << "]";
		qDebug() << "[ N x P ] = [" << N2 << "x" << P << "]";

		qDebug() << "Input Matrices have incompatible dimensions!";
		qDebug() << "Please use the following format [ M × N ] × [ N × P ]";
		exit (EXIT_FAILURE);
	}
	else
		N = N1;
}


bool argsAreValid( char * arg1, char * arg2, char * arg3 ){
	bool incorrect = false;
	
	QFileInfo input1( arg1 );
	QFileInfo input2( arg2 );
	QFileInfo output( arg3 );

	if( !input1.isFile() || !input1.isReadable() ){
		qDebug() << arg1 << " is not a readable file";
		incorrect = true;
	}
	if ( !input2.isFile() || !input2.isReadable()  ){
		qDebug() << arg2 << " is not a readable file";
		incorrect = true;
	}
	if ( !output.isFile() || !output.isWritable() ){
		qDebug() << arg3 << " is not a writable file";
		if(!output.exists()){
			qDebug() << arg3 << " does not exist!";
		}
		incorrect = true;
	}
	 
	if(!incorrect){
		incorrect = false;
	}

	return incorrect;
}