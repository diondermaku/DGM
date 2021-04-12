#include "DNN.h"
#include "DGM/timer.h"

namespace dgm = DirectGraphicalModels;

void backPropagate(std::vector<dgm::dnn::ptr_neuron_t>& vpLayerA,
                   std::vector<dgm::dnn::ptr_neuron_t>& vpLayerB,
                   std::vector<dgm::dnn::ptr_neuron_t>& vpLayerC,
                   double resultErrorRate[]);

void dotProd(std::vector<dgm::dnn::ptr_neuron_t>& vpLayerA, std::vector<dgm::dnn::ptr_neuron_t>& vpLayerB);

/**
 * Applies the Sigmoid Activation function
 *
 * @param the value at each node
 * @return a number between 0 and 1.
 */
float applySigmoidFunction(float val)
{
	float sigmoid = 1 / (1 + exp(-val));
	return sigmoid;
}

/**
 * Reads the digits numerical value in a decimal notation
 *
 * @param file to read, and the number of digits to read
 * @return an array of digit labels
 */
std::vector<byte> readGroundTruth(const std::string& fileName)
{
	std::vector<byte> res;
	std::ifstream inFile;
	inFile.open(fileName.c_str());

	if (inFile.is_open()) {
		int val;
		while (!inFile.eof()) {
			inFile >> val;
			res.push_back(static_cast<byte>(val));
		}
		inFile.close();
	}
	return res;
}

/**
 * Reads the image pixel value
 *
 * @param image to read, and the number of images to read
 * @return an array of pixel values for each image
 */
int **readImgData(std::string file, int dataSize)
{
	const int inputLayer = 784;
	static int **trainDataBin = new int*[dataSize];
	
	for(int m = 0; m < dataSize; m++)
	{
		trainDataBin[m] = new int[inputLayer];
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(4);
		ss << m;
		std::string number = ss.str();
		std::string path = file + number + ".png";
		std::string image_path = samples::findFile(path);

		Mat img = imread(image_path, 0);
		
		int l=0;
		for(int i = 0; i < img.rows; i++) {
			for(int j = 0; j < img.cols; j++) {
				int value = abs((int)img.at<uchar>(i,j) - 255);
				trainDataBin[m][l] = value;
				l++;
			}
		}
	}
	return trainDataBin;
}


int main() {
    const size_t     numNeuronsInputLayer   = 784;
    const size_t     numNeuronsHiddenLayer  = 60;
    const size_t     numNeuronsOutputLayer  = 10;
    const size_t                  dataSize  = 4000;

    std::vector<dgm::dnn::ptr_neuron_t> vpInputLayer;
    std::vector<dgm::dnn::ptr_neuron_t> vpHiddenLayer;
    std::vector<dgm::dnn::ptr_neuron_t> vpOutputLayer;

    for (size_t i = 0; i < numNeuronsInputLayer; i++)
        vpInputLayer.push_back( std::make_shared<dgm::dnn::CNeuron>(numNeuronsHiddenLayer, 0) );

    for (size_t i = 0; i < numNeuronsHiddenLayer; i++)
        vpHiddenLayer.push_back( std::make_shared<dgm::dnn::CNeuron>(numNeuronsOutputLayer) );

    for (size_t i = 0; i < numNeuronsOutputLayer; i++)
        vpOutputLayer.push_back( std::make_shared<dgm::dnn::CNeuron>(0) );

    int **trainDataBin   = readImgData("../../../data/digits/train/digit_", dataSize);
    auto trainDataDigit  = readGroundTruth("../../../data/digits/train_gt.txt");
	assert(trainDataDigit.size() == dataSize);
	
    for (size_t i = 0; i < vpHiddenLayer.size(); i++)
        vpHiddenLayer[i]->generateRandomWeights();

    for(size_t i = 0; i < vpInputLayer.size(); i++)
        vpInputLayer[i]->generateRandomWeights();

	dgm::Timer::start("Training...");
	for(int k = 0; k < dataSize; k++) {
        for(size_t i = 0; i < vpInputLayer.size(); i++) {
            float val = (float)trainDataBin[k][i]/255;
            vpInputLayer[i]->setNodeValue(val);
        }

        dotProd(vpInputLayer, vpHiddenLayer);
        dotProd(vpHiddenLayer, vpOutputLayer);
    
        double *resultErrorRate = new double[numNeuronsOutputLayer];
		for(size_t i = 0; i < vpOutputLayer.size(); i++) {
			resultErrorRate[i] = (trainDataDigit[k] == i) ? 1 : 0;
			resultErrorRate[i] -= vpOutputLayer[i]->getNodeValue();
		}


        backPropagate(vpInputLayer, vpHiddenLayer, vpOutputLayer, resultErrorRate);
    }
	dgm::Timer::stop();

//     ==================== TEST DIGITS ====================
    int testDataSize    = 2000;
    int correct         = 0;
    int uncorrect       = 0;
    int **testDataBin   = readImgData("../../../data/digits/test/digit_", testDataSize);
    auto testDataDigit  = readGroundTruth("../../../data/digits/test_gt.txt");

    
//    for (auto i: testDataDigit)
//        printf("%d ", i);
    
    
	dgm::Timer::start("Testing...");
	for(size_t z = 0; z < testDataSize; z++) {
		 for(size_t i = 0; i < vpInputLayer.size(); i++) {
			 float val = (float)testDataBin[z][i]/255;
			 vpInputLayer[i]->setNodeValue(val);
		 }

		 dotProd(vpInputLayer, vpHiddenLayer);
		 dotProd(vpHiddenLayer, vpOutputLayer);

		 double *allPredictionsforDigits = new double[numNeuronsOutputLayer];
		 for(size_t i=0 ; i < vpOutputLayer.size(); i++) {
			 allPredictionsforDigits[i] = vpOutputLayer[i]->getNodeValue();
		 }

		 float maxAccuracy = 0;
		 int   number;
		 for(size_t i=0 ; i < vpOutputLayer.size(); i++) {
			 if(allPredictionsforDigits[i] >= maxAccuracy) {
				 maxAccuracy = allPredictionsforDigits[i];
				 number = i;
			 }
		 }
        //printf("prediction [%d] for digit %d with %.3f%s at position %zu \n", number, testDataDigit[z], maxAccuracy, "%", z);
        number == testDataDigit[z] ? correct++ : uncorrect++;
	}
	dgm::Timer::stop();

    printf("poz: %d\nneg: %d\n", correct, uncorrect);
    printf("average: %.2f%s\n", (float)correct/(correct+uncorrect)*100, "%");
	return 0;
}

void dotProd(std::vector<dgm::dnn::ptr_neuron_t>& vpLayerA, std::vector<dgm::dnn::ptr_neuron_t>& vpLayerB) {
	for(size_t i = 0 ; i < vpLayerB.size(); i++) {
        float value = 0;
        for(const auto& a : vpLayerA)
            value += a->getWeight(i) * a->getNodeValue();

        value = applySigmoidFunction(value);
        vpLayerB[i]->setNodeValue(value);
    }
}

void backPropagate(std::vector<dgm::dnn::ptr_neuron_t>& vpLayerA,
                   std::vector<dgm::dnn::ptr_neuron_t>& vpLayerB,
                   std::vector<dgm::dnn::ptr_neuron_t>& vpLayerC,
                   double resultErrorRate[])
{

    const int numNeuronsInputLayer = 784;
    const int numNeuronsHiddenLayer = 60;
    const int numNeuronsOutputLayer = 10;
    
    float (*DeltaWjk)[numNeuronsOutputLayer]  = new float[numNeuronsHiddenLayer][numNeuronsOutputLayer];
    float (*DeltaWik)[numNeuronsHiddenLayer]  = new float[numNeuronsInputLayer][numNeuronsHiddenLayer];
    float *DeltaJ                             = new float[numNeuronsHiddenLayer];
    float learningRate                        = 0.1;

    for(size_t i = 0; i < vpLayerB.size(); i++) {
        double nodeVal = 0;
        for(size_t j = 0; j < vpLayerC.size(); j++) {
            nodeVal += vpLayerB[i]->getWeight(j) * resultErrorRate[j];
            DeltaWjk[i][j] = learningRate * resultErrorRate[j]* vpLayerB[i]->getNodeValue();
        }
        float sigmoid = applySigmoidFunction(vpLayerB[i]->getNodeValue());
        DeltaJ[i] = nodeVal * sigmoid * (1-sigmoid);
    }

    for(size_t i = 0; i < vpLayerA.size(); i++) {
        for(size_t j = 0; j < vpLayerB.size(); j++) {
            DeltaWik[i][j] = learningRate * DeltaJ[j] * vpLayerA[i]->getNodeValue();
            float oldWeight = vpLayerA[i]->getWeight(j);
            vpLayerA[i]->setWeight(j, oldWeight + DeltaWik[i][j]);
        }
    }

    for(size_t i = 0; i < vpLayerB.size(); i++) {
        for(size_t j = 0; j < vpLayerC.size(); j++) {
            float oldWeight = vpLayerB[i]->getWeight(j);
            vpLayerB[i]->setWeight(j, oldWeight + DeltaWjk[i][j]);
        }
    }
}
