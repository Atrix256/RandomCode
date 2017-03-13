#include <stdio.h>
#include <array>

// Nonzero value enables csv logging.
#define LOG_TO_CSV_NUMSAMPLES() 50

// ===== Example 1 - One Neuron, One training Example =====

void Example1RunNetwork (
	float input, float desiredOutput,
	float weight, float bias,
	float& error, float& cost, float& actualOutput,
	float& deltaCost_deltaWeight, float& deltaCost_deltaBias, float& deltaCost_deltaInput
) {
	// calculate Z (weighted input) and O (activation function of weighted input) for the neuron
	float Z = input * weight + bias;
	float O = 1.0f / (1.0f + std::exp(-Z));

	// the actual output of the network is the activation of the neuron
	actualOutput = O;

	// calculate error
	error = std::abs(desiredOutput - actualOutput);

	// calculate cost
	cost = 0.5f * error * error;

	// calculate how much a change in neuron activation affects the cost function
	// deltaCost/deltaO = O - target
	float deltaCost_deltaO = O - desiredOutput;

	// calculate how much a change in neuron weighted input affects neuron activation
	// deltaO/deltaZ = O * (1 - O)
	float deltaO_deltaZ = O * (1 - O);


	// calculate how much a change in a neuron's weighted input affects the cost function.
	// This is deltaCost/deltaZ, which equals deltaCost/deltaO * deltaO/deltaZ
	// This is also deltaCost/deltaBias and is also refered to as the error of the neuron
	float neuronError = deltaCost_deltaO * deltaO_deltaZ;
	deltaCost_deltaBias = neuronError;

	// calculate how much a change in the weight affects the cost function.
	// deltaCost/deltaWeight = deltaCost/deltaO * deltaO/deltaZ * deltaZ/deltaWeight
	// deltaCost/deltaWeight = neuronError * deltaZ/deltaWeight
	// deltaCost/deltaWeight = neuronError * input
	deltaCost_deltaWeight = neuronError * input;


	// As a bonus, calculate how much a change in the input affects the cost function.
	// Follows same logic as deltaCost/deltaWeight, but deltaZ/deltaInput is the weight.
	// deltaCost/deltaInput = neuronError * weight
	deltaCost_deltaInput = neuronError * weight;
}

void Example1 ()
{
	#if LOG_TO_CSV_NUMSAMPLES() > 0
		// open the csv file for this example
		FILE *file = fopen("Example1.csv","w+t");
		if (file != nullptr)
			fprintf(file, "\"training index\",\"error\",\"cost\",\"weight\",\"bias\",\"dCost/dWeight\",\"dCost/dBias\",\"dCost/dInput\"\n");
	#endif

	// learning parameters for the network
	const float c_learningRate = 0.5f;
	const size_t c_numTrainings = 10000;

	// training data
	// input: 1, output: 0
	const std::array<float, 2> c_trainingData = {1.0f, 0.0f};

	// starting weight and bias values
	float weight = 0.3f;
	float bias = 0.5f;

	// iteratively train the network
	float error = 0.0f;
	for (size_t trainingIndex = 0; trainingIndex < c_numTrainings; ++trainingIndex)
	{
		// run the network to get error and derivatives
		float output = 0.0f;
		float cost = 0.0f;
		float deltaCost_deltaWeight = 0.0f;
		float deltaCost_deltaBias = 0.0f;
		float deltaCost_deltaInput = 0.0f;
		Example1RunNetwork(c_trainingData[0], c_trainingData[1], weight, bias, error, cost, output, deltaCost_deltaWeight, deltaCost_deltaBias, deltaCost_deltaInput);

		#if LOG_TO_CSV_NUMSAMPLES() > 0
			const size_t trainingInterval = (c_numTrainings / (LOG_TO_CSV_NUMSAMPLES() - 1));
			if (file != nullptr && (trainingIndex % trainingInterval == 0 || trainingIndex == c_numTrainings - 1))
			{
				// log to the csv
				fprintf(file, "\"%zi\",\"%f\",\"%f\",\"%f\",\"%f\",\"%f\",\"%f\",\"%f\",\n", trainingIndex, error, cost, weight, bias, deltaCost_deltaWeight, deltaCost_deltaBias, deltaCost_deltaInput);
			}
		#endif

		// adjust weights and biases
		weight -= deltaCost_deltaWeight * c_learningRate;
		bias -= deltaCost_deltaBias * c_learningRate;
	}

	printf("Example1 Final Error: %f\n", error);

	#if LOG_TO_CSV_NUMSAMPLES() > 0
		if (file != nullptr)
			fclose(file);
	#endif
}

// ===== Example 2 - One Neuron, Two training Examples =====

void Example2 ()
{
	#if LOG_TO_CSV_NUMSAMPLES() > 0
		// open the csv file for this example
		FILE *file = fopen("Example2.csv","w+t");
		if (file != nullptr)
			fprintf(file, "\"training index\",\"error\",\"cost\",\"weight\",\"bias\",\"dCost/dWeight\",\"dCost/dBias\",\"dCost/dInput\"\n");
	#endif

	// learning parameters for the network
	const float c_learningRate = 0.5f;
	const size_t c_numTrainings = 100000;

	// training data
	// input: 1, output: 0
	// input: 0, output: 1
	const std::array<std::array<float, 2>, 2> c_trainingData = { {
		{1.0f, 0.0f},
		{0.0f, 1.0f}
	} };

	// starting weight and bias values
	float weight = 0.3f;
	float bias = 0.5f;

	// iteratively train the network
	float avgError = 0.0f;
	for (size_t trainingIndex = 0; trainingIndex < c_numTrainings; ++trainingIndex)
	{
		avgError = 0.0f;
		float avgOutput = 0.0f;
		float avgCost = 0.0f;
		float avgDeltaCost_deltaWeight = 0.0f;
		float avgDeltaCost_deltaBias = 0.0f;
		float avgDeltaCost_deltaInput = 0.0f;

		// run the network to get error and derivatives for each training example
		for (const std::array<float, 2>& trainingData : c_trainingData)
		{
			float error = 0.0f;
			float output = 0.0f;
			float cost = 0.0f;
			float deltaCost_deltaWeight = 0.0f;
			float deltaCost_deltaBias = 0.0f;
			float deltaCost_deltaInput = 0.0f;
			Example1RunNetwork(trainingData[0], trainingData[1], weight, bias, error, cost, output, deltaCost_deltaWeight, deltaCost_deltaBias, deltaCost_deltaInput);

			avgError += error;
			avgOutput += output;
			avgCost += cost;
			avgDeltaCost_deltaWeight += deltaCost_deltaWeight;
			avgDeltaCost_deltaBias += deltaCost_deltaBias;
			avgDeltaCost_deltaInput += deltaCost_deltaInput;
		}

		avgError /= (float)c_trainingData.size();
		avgOutput /= (float)c_trainingData.size();
		avgCost /= (float)c_trainingData.size();
		avgDeltaCost_deltaWeight /= (float)c_trainingData.size();
		avgDeltaCost_deltaBias /= (float)c_trainingData.size();
		avgDeltaCost_deltaInput /= (float)c_trainingData.size();

		#if LOG_TO_CSV_NUMSAMPLES() > 0
			const size_t trainingInterval = (c_numTrainings / (LOG_TO_CSV_NUMSAMPLES() - 1));
			if (file != nullptr && (trainingIndex % trainingInterval == 0 || trainingIndex == c_numTrainings - 1))
			{
				// log to the csv
				fprintf(file, "\"%zi\",\"%f\",\"%f\",\"%f\",\"%f\",\"%f\",\"%f\",\"%f\",\n", trainingIndex, avgError, avgCost, weight, bias, avgDeltaCost_deltaWeight, avgDeltaCost_deltaBias, avgDeltaCost_deltaInput);
			}
		#endif

		// adjust weights and biases
		weight -= avgDeltaCost_deltaWeight * c_learningRate;
		bias -= avgDeltaCost_deltaBias * c_learningRate;
	}

	printf("Example2 Final Error: %f\n", avgError);

	#if LOG_TO_CSV_NUMSAMPLES() > 0
		if (file != nullptr)
			fclose(file);
	#endif
}

// ===== Example 3 - Two inputs, two neurons in one layer =====

struct SExample3Training
{
	std::array<float, 2> m_input;
	std::array<float, 2> m_output;
};

void Example3RunNetwork (
	const std::array<float, 2>& input, const std::array<float, 2>& desiredOutput,
	const std::array<float, 4>& weights, const std::array<float, 2>& biases,
	float& error, float& cost, std::array<float, 2>& actualOutput,
	std::array<float, 4>& deltaCost_deltaWeights, std::array<float, 2>& deltaCost_deltaBiases, std::array<float, 2>& deltaCost_deltaInputs
) {

	// calculate Z0 and O0 for neuron0
	float Z0 = input[0] * weights[0] + input[1] * weights[1] + biases[0];
	float O0 = 1.0f / (1.0f + std::exp(-Z0));

	// calculate Z1 and O1 for neuron1
	float Z1 = input[0] * weights[2] + input[1] * weights[3] + biases[1];
	float O1 = 1.0f / (1.0f + std::exp(-Z1));

	// the actual output of the network is the activation of the neurons
	actualOutput[0] = O0;
	actualOutput[1] = O1;

	// calculate error
	float diff0 = desiredOutput[0] - actualOutput[0];
	float diff1 = desiredOutput[1] - actualOutput[1];
	error = std::sqrt(diff0*diff0 + diff1*diff1);

	// calculate cost
	cost = 0.5f * error * error;

	//----- Neuron 0 -----

	// calculate how much a change in neuron 0 activation affects the cost function
	// deltaCost/deltaO0 = O0 - target0
	float deltaCost_deltaO0 = O0 - desiredOutput[0];

	// calculate how much a change in neuron 0 weighted input affects neuron 0 activation
	// deltaO0/deltaZ0 = O0 * (1 - O0)
	float deltaO0_deltaZ0 = O0 * (1 - O0);

	// calculate how much a change in neuron 0 weighted input affects the cost function.
	// This is deltaCost/deltaZ0, which equals deltaCost/deltaO0 * deltaO0/deltaZ0
	// This is also deltaCost/deltaBias0 and is also refered to as the error of neuron 0
	float neuron0Error = deltaCost_deltaO0 * deltaO0_deltaZ0;
	deltaCost_deltaBiases[0] = neuron0Error;

	// calculate how much a change in weight0 affects the cost function.
	// deltaCost/deltaWeight0 = deltaCost/deltaO0 * deltaO/deltaZ0 * deltaZ0/deltaWeight0
	// deltaCost/deltaWeight0 = neuron0Error * deltaZ/deltaWeight0
	// deltaCost/deltaWeight0 = neuron0Error * input0
	// similar thing for weight1
	deltaCost_deltaWeights[0] = neuron0Error * input[0];
	deltaCost_deltaWeights[1] = neuron0Error * input[1];

	//----- Neuron 1 -----

	// calculate how much a change in neuron 1 activation affects the cost function
	// deltaCost/deltaO1 = O1 - target1
	float deltaCost_deltaO1 = O1 - desiredOutput[1];

	// calculate how much a change in neuron 1 weighted input affects neuron 1 activation
	// deltaO0/deltaZ1 = O1 * (1 - O1)
	float deltaO1_deltaZ1 = O1 * (1 - O1);

	// calculate how much a change in neuron 1 weighted input affects the cost function.
	// This is deltaCost/deltaZ1, which equals deltaCost/deltaO1 * deltaO1/deltaZ1
	// This is also deltaCost/deltaBias1 and is also refered to as the error of neuron 1
	float neuron1Error = deltaCost_deltaO1 * deltaO1_deltaZ1;
	deltaCost_deltaBiases[1] = neuron1Error;

	// calculate how much a change in weight2 affects the cost function.
	// deltaCost/deltaWeight2 = deltaCost/deltaO1 * deltaO/deltaZ1 * deltaZ0/deltaWeight1
	// deltaCost/deltaWeight2 = neuron1Error * deltaZ/deltaWeight1
	// deltaCost/deltaWeight2 = neuron1Error * input0
	// similar thing for weight3
	deltaCost_deltaWeights[2] = neuron1Error * input[0];
	deltaCost_deltaWeights[3] = neuron1Error * input[1];

	//----- Input -----

	// As a bonus, calculate how much a change in the inputs affect the cost function.
	// A complication here compared to Example1 and Example2 is that each input affects two neurons instead of only one.
	// That means that...
	// deltaCost/deltaInput0 = deltaCost/deltaZ0 * deltaZ0/deltaInput0 + deltaCost/deltaZ1 * deltaZ1/deltaInput0
	//                       = neuron0Error * weight0 + neuron1Error * weight2
	// and
	// deltaCost/deltaInput1 = deltaCost/deltaZ0 * deltaZ0/deltaInput1 + deltaCost/deltaZ1 * deltaZ1/deltaInput1
	//                       = neuron0Error * weight1 + neuron1Error * weight3
	deltaCost_deltaInputs[0] = neuron0Error * weights[0] + neuron1Error * weights[2];
	deltaCost_deltaInputs[1] = neuron0Error * weights[1] + neuron1Error * weights[3];
}

void Example3 ()
{
	#if LOG_TO_CSV_NUMSAMPLES() > 0
		// open the csv file for this example
		FILE *file = fopen("Example3.csv","w+t");
		if (file != nullptr)
			fprintf(file, "\"training index\",\"error\",\"cost\"\n");
	#endif

	// learning parameters for the network
	const float c_learningRate = 0.5f;
	const size_t c_numTrainings = 520000;

	// training data: OR/AND
	// input: 00, output: 00
	// input: 01, output: 10
	// input: 10, output: 10
	// input: 11, output: 11
	const std::array<SExample3Training, 4> c_trainingData = { {
		{{0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.0f, 1.0f}, {1.0f, 0.0f}},
		{{1.0f, 0.0f}, {1.0f, 0.0f}},
		{{1.0f, 1.0f}, {1.0f, 1.0f}},
	} };

	// starting weight and bias values
	std::array<float, 4> weights = { 0.2f, 0.8f, 0.6f, 0.4f };
	std::array<float, 2> biases = { 0.5f, 0.1f };

	// iteratively train the network
	float avgError = 0.0f;
	for (size_t trainingIndex = 0; trainingIndex < c_numTrainings; ++trainingIndex)
	{
		//float avgCost = 0.0f;
		std::array<float, 2> avgOutput = { 0.0f, 0.0f };
		std::array<float, 4> avgDeltaCost_deltaWeights = { 0.0f, 0.0f, 0.0f, 0.0f };
		std::array<float, 2> avgDeltaCost_deltaBiases = { 0.0f, 0.0f };
		std::array<float, 2> avgDeltaCost_deltaInputs = { 0.0f, 0.0f };
		avgError = 0.0f;
		float avgCost = 0.0;

		// run the network to get error and derivatives for each training example
		for (const SExample3Training& trainingData : c_trainingData)
		{
			float error = 0.0f;
			std::array<float, 2> output = { 0.0f, 0.0f };
			float cost = 0.0f;
			std::array<float, 4> deltaCost_deltaWeights = { 0.0f, 0.0f, 0.0f, 0.0f };
			std::array<float, 2> deltaCost_deltaBiases = { 0.0f, 0.0f };
			std::array<float, 2> deltaCost_deltaInputs = { 0.0f, 0.0f };
			Example3RunNetwork(trainingData.m_input, trainingData.m_output, weights, biases, error, cost, output, deltaCost_deltaWeights, deltaCost_deltaBiases, deltaCost_deltaInputs);

			avgError += error;
			avgCost += cost;
			for (size_t i = 0; i < avgOutput.size(); ++i)
				avgOutput[i] += output[i];
			for (size_t i = 0; i < avgDeltaCost_deltaWeights.size(); ++i)
				avgDeltaCost_deltaWeights[i] += deltaCost_deltaWeights[i];
			for (size_t i = 0; i < avgDeltaCost_deltaBiases.size(); ++i)
				avgDeltaCost_deltaBiases[i] += deltaCost_deltaBiases[i];
			for (size_t i = 0; i < avgDeltaCost_deltaInputs.size(); ++i)
				avgDeltaCost_deltaInputs[i] += deltaCost_deltaInputs[i];
		}

		avgError /= (float)c_trainingData.size();
		avgCost /= (float)c_trainingData.size();
		for (size_t i = 0; i < avgOutput.size(); ++i)
			avgOutput[i] /= (float)c_trainingData.size();
		for (size_t i = 0; i < avgDeltaCost_deltaWeights.size(); ++i)
			avgDeltaCost_deltaWeights[i] /= (float)c_trainingData.size();
		for (size_t i = 0; i < avgDeltaCost_deltaBiases.size(); ++i)
			avgDeltaCost_deltaBiases[i] /= (float)c_trainingData.size();
		for (size_t i = 0; i < avgDeltaCost_deltaInputs.size(); ++i)
			avgDeltaCost_deltaInputs[i] /= (float)c_trainingData.size();

		#if LOG_TO_CSV_NUMSAMPLES() > 0
			const size_t trainingInterval = (c_numTrainings / (LOG_TO_CSV_NUMSAMPLES() - 1));
			if (file != nullptr && (trainingIndex % trainingInterval == 0 || trainingIndex == c_numTrainings - 1))
			{
				// log to the csv
				fprintf(file, "\"%zi\",\"%f\",\"%f\"\n", trainingIndex, avgError, avgCost);
			}
		#endif

		// adjust weights and biases
		for (size_t i = 0; i < weights.size(); ++i)
			weights[i] -= avgDeltaCost_deltaWeights[i] * c_learningRate;
		for (size_t i = 0; i < biases.size(); ++i)
			biases[i] -= avgDeltaCost_deltaBiases[i] * c_learningRate;
	}

	printf("Example3 Final Error: %f\n", avgError);

	#if LOG_TO_CSV_NUMSAMPLES() > 0
		if (file != nullptr)
			fclose(file);
	#endif
}

// ===== Example 4 - Two layers with two neurons in each layer =====

void Example4RunNetwork (
	const std::array<float, 2>& input, const std::array<float, 2>& desiredOutput,
	const std::array<float, 8>& weights, const std::array<float, 4>& biases,
	float& error, float& cost, std::array<float, 2>& actualOutput,
	std::array<float, 8>& deltaCost_deltaWeights, std::array<float, 4>& deltaCost_deltaBiases, std::array<float, 2>& deltaCost_deltaInputs
) {
	// calculate Z0 and O0 for neuron0
	float Z0 = input[0] * weights[0] + input[1] * weights[1] + biases[0];
	float O0 = 1.0f / (1.0f + std::exp(-Z0));

	// calculate Z1 and O1 for neuron1
	float Z1 = input[0] * weights[2] + input[1] * weights[3] + biases[1];
	float O1 = 1.0f / (1.0f + std::exp(-Z1));

	// calculate Z2 and O2 for neuron2
	float Z2 = O0 * weights[4] + O1 * weights[5] + biases[2];
	float O2 = 1.0f / (1.0f + std::exp(-Z2));

	// calculate Z3 and O3 for neuron3
	float Z3 = O0 * weights[6] + O1 * weights[7] + biases[3];
	float O3 = 1.0f / (1.0f + std::exp(-Z3));

	// the actual output of the network is the activation of the output layer neurons
	actualOutput[0] = O2;
	actualOutput[1] = O3;

	// calculate error
	float diff0 = desiredOutput[0] - actualOutput[0];
	float diff1 = desiredOutput[1] - actualOutput[1];
	error = std::sqrt(diff0*diff0 + diff1*diff1);

	// calculate cost
	cost = 0.5f * error * error;

	//----- Neuron 2 -----

	// calculate how much a change in neuron 2 activation affects the cost function
	// deltaCost/deltaO2 = O2 - target0
	float deltaCost_deltaO2 = O2 - desiredOutput[0];

	// calculate how much a change in neuron 2 weighted input affects neuron 2 activation
	// deltaO2/deltaZ2 = O2 * (1 - O2)
	float deltaO2_deltaZ2 = O2 * (1 - O2);

	// calculate how much a change in neuron 2 weighted input affects the cost function.
	// This is deltaCost/deltaZ2, which equals deltaCost/deltaO2 * deltaO2/deltaZ2
	// This is also deltaCost/deltaBias2 and is also refered to as the error of neuron 2
	float neuron2Error = deltaCost_deltaO2 * deltaO2_deltaZ2;
	deltaCost_deltaBiases[2] = neuron2Error;

	// calculate how much a change in weight4 affects the cost function.
	// deltaCost/deltaWeight4 = deltaCost/deltaO2 * deltaO2/deltaZ2 * deltaZ2/deltaWeight4
	// deltaCost/deltaWeight4 = neuron2Error * deltaZ/deltaWeight4
	// deltaCost/deltaWeight4 = neuron2Error * O0
	// similar thing for weight5
	deltaCost_deltaWeights[4] = neuron2Error * O0;
	deltaCost_deltaWeights[5] = neuron2Error * O1;

	//----- Neuron 3 -----

	// calculate how much a change in neuron 3 activation affects the cost function
	// deltaCost/deltaO3 = O3 - target1
	float deltaCost_deltaO3 = O3 - desiredOutput[1];

	// calculate how much a change in neuron 3 weighted input affects neuron 3 activation
	// deltaO3/deltaZ3 = O3 * (1 - O3)
	float deltaO3_deltaZ3 = O3 * (1 - O3);

	// calculate how much a change in neuron 3 weighted input affects the cost function.
	// This is deltaCost/deltaZ3, which equals deltaCost/deltaO3 * deltaO3/deltaZ3
	// This is also deltaCost/deltaBias3 and is also refered to as the error of neuron 3
	float neuron3Error = deltaCost_deltaO3 * deltaO3_deltaZ3;
	deltaCost_deltaBiases[3] = neuron3Error;

	// calculate how much a change in weight6 affects the cost function.
	// deltaCost/deltaWeight6 = deltaCost/deltaO3 * deltaO3/deltaZ3 * deltaZ3/deltaWeight6
	// deltaCost/deltaWeight6 = neuron3Error * deltaZ/deltaWeight6
	// deltaCost/deltaWeight6 = neuron3Error * O0
	// similar thing for weight7
	deltaCost_deltaWeights[6] = neuron3Error * O0;
	deltaCost_deltaWeights[7] = neuron3Error * O1;

	//----- Neuron 0 -----

	// calculate how much a change in neuron 0 activation affects the cost function
	// deltaCost/deltaO0 = deltaCost/deltaZ2 * deltaZ2/deltaO0 + deltaCost/deltaZ3 * deltaZ3/deltaO0
	// deltaCost/deltaO0 = neuron2Error * weight4 + neuron3error * weight6
	float deltaCost_deltaO0 = neuron2Error * weights[4] + neuron3Error * weights[6];

	// calculate how much a change in neuron 0 weighted input affects neuron 0 activation
	// deltaO0/deltaZ0 = O0 * (1 - O0)
	float deltaO0_deltaZ0 = O0 * (1 - O0);

	// calculate how much a change in neuron 0 weighted input affects the cost function.
	// This is deltaCost/deltaZ0, which equals deltaCost/deltaO0 * deltaO0/deltaZ0
	// This is also deltaCost/deltaBias0 and is also refered to as the error of neuron 0
	float neuron0Error = deltaCost_deltaO0 * deltaO0_deltaZ0;
	deltaCost_deltaBiases[0] = neuron0Error;

	// calculate how much a change in weight0 affects the cost function.
	// deltaCost/deltaWeight0 = deltaCost/deltaO0 * deltaO0/deltaZ0 * deltaZ0/deltaWeight0
	// deltaCost/deltaWeight0 = neuron0Error * deltaZ0/deltaWeight0
	// deltaCost/deltaWeight0 = neuron0Error * input0
	// similar thing for weight1
	deltaCost_deltaWeights[0] = neuron0Error * input[0];
	deltaCost_deltaWeights[1] = neuron0Error * input[1];

	//----- Neuron 1 -----

	// calculate how much a change in neuron 1 activation affects the cost function
	// deltaCost/deltaO1 = deltaCost/deltaZ2 * deltaZ2/deltaO1 + deltaCost/deltaZ3 * deltaZ3/deltaO1
	// deltaCost/deltaO1 = neuron2Error * weight5 + neuron3error * weight7
	float deltaCost_deltaO1 = neuron2Error * weights[5] + neuron3Error * weights[7];

	// calculate how much a change in neuron 1 weighted input affects neuron 1 activation
	// deltaO1/deltaZ1 = O1 * (1 - O1)
	float deltaO1_deltaZ1 = O1 * (1 - O1);

	// calculate how much a change in neuron 1 weighted input affects the cost function.
	// This is deltaCost/deltaZ1, which equals deltaCost/deltaO1 * deltaO1/deltaZ1
	// This is also deltaCost/deltaBias1 and is also refered to as the error of neuron 1
	float neuron1Error = deltaCost_deltaO1 * deltaO1_deltaZ1;
	deltaCost_deltaBiases[1] = neuron1Error;

	// calculate how much a change in weight2 affects the cost function.
	// deltaCost/deltaWeight2 = deltaCost/deltaO1 * deltaO1/deltaZ1 * deltaZ1/deltaWeight2
	// deltaCost/deltaWeight2 = neuron1Error * deltaZ2/deltaWeight2
	// deltaCost/deltaWeight2 = neuron1Error * input0
	// similar thing for weight3
	deltaCost_deltaWeights[2] = neuron1Error * input[0];
	deltaCost_deltaWeights[3] = neuron1Error * input[1];

	//----- Input -----

	// As a bonus, calculate how much a change in the inputs affect the cost function.
	// A complication here compared to Example1 and Example2 is that each input affects two neurons instead of only one.
	// That means that...
	// deltaCost/deltaInput0 = deltaCost/deltaZ0 * deltaZ0/deltaInput0 + deltaCost/deltaZ1 * deltaZ1/deltaInput0
	//                       = neuron0Error * weight0 + neuron1Error * weight2
	// and
	// deltaCost/deltaInput1 = deltaCost/deltaZ0 * deltaZ0/deltaInput1 + deltaCost/deltaZ1 * deltaZ1/deltaInput1
	//                       = neuron0Error * weight1 + neuron1Error * weight3
	deltaCost_deltaInputs[0] = neuron0Error * weights[0] + neuron1Error * weights[2];
	deltaCost_deltaInputs[1] = neuron0Error * weights[1] + neuron1Error * weights[3];
}

void Example4 ()
{
	#if LOG_TO_CSV_NUMSAMPLES() > 0
		// open the csv file for this example
		FILE *file = fopen("Example4.csv","w+t");
		if (file != nullptr)
			fprintf(file, "\"training index\",\"error\",\"cost\"\n");
	#endif

	// learning parameters for the network
	const float c_learningRate = 0.5f;
	const size_t c_numTrainings = 5000;

	// training data: 0.05, 0.1 in = 0.01, 0.99 out
	const std::array<SExample3Training, 1> c_trainingData = { {
		{{0.05f, 0.1f}, {0.01f, 0.99f}},
	} };

	// starting weight and bias values
	std::array<float, 8> weights = { 0.15f, 0.2f, 0.25f, 0.3f, 0.4f, 0.45f, 0.5f, 0.55f};
	std::array<float, 4> biases = { 0.35f, 0.35f, 0.6f, 0.6f };

	// iteratively train the network
	float avgError = 0.0f;
	for (size_t trainingIndex = 0; trainingIndex < c_numTrainings; ++trainingIndex)
	{
		std::array<float, 2> avgOutput = { 0.0f, 0.0f };
		std::array<float, 8> avgDeltaCost_deltaWeights = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		std::array<float, 4> avgDeltaCost_deltaBiases = { 0.0f, 0.0f, 0.0f, 0.0f };
		std::array<float, 2> avgDeltaCost_deltaInputs = { 0.0f, 0.0f };
		avgError = 0.0f;
		float avgCost = 0.0;

		// run the network to get error and derivatives for each training example
		for (const SExample3Training& trainingData : c_trainingData)
		{
			float error = 0.0f;
			std::array<float, 2> output = { 0.0f, 0.0f };
			float cost = 0.0f;
			std::array<float, 8> deltaCost_deltaWeights = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
			std::array<float, 4> deltaCost_deltaBiases = { 0.0f, 0.0f, 0.0f, 0.0f };
			std::array<float, 2> deltaCost_deltaInputs = { 0.0f, 0.0f };
			Example4RunNetwork(trainingData.m_input, trainingData.m_output, weights, biases, error, cost, output, deltaCost_deltaWeights, deltaCost_deltaBiases, deltaCost_deltaInputs);

			avgError += error;
			avgCost += cost;
			for (size_t i = 0; i < avgOutput.size(); ++i)
				avgOutput[i] += output[i];
			for (size_t i = 0; i < avgDeltaCost_deltaWeights.size(); ++i)
				avgDeltaCost_deltaWeights[i] += deltaCost_deltaWeights[i];
			for (size_t i = 0; i < avgDeltaCost_deltaBiases.size(); ++i)
				avgDeltaCost_deltaBiases[i] += deltaCost_deltaBiases[i];
			for (size_t i = 0; i < avgDeltaCost_deltaInputs.size(); ++i)
				avgDeltaCost_deltaInputs[i] += deltaCost_deltaInputs[i];
		}

		avgError /= (float)c_trainingData.size();
		avgCost /= (float)c_trainingData.size();
		for (size_t i = 0; i < avgOutput.size(); ++i)
			avgOutput[i] /= (float)c_trainingData.size();
		for (size_t i = 0; i < avgDeltaCost_deltaWeights.size(); ++i)
			avgDeltaCost_deltaWeights[i] /= (float)c_trainingData.size();
		for (size_t i = 0; i < avgDeltaCost_deltaBiases.size(); ++i)
			avgDeltaCost_deltaBiases[i] /= (float)c_trainingData.size();
		for (size_t i = 0; i < avgDeltaCost_deltaInputs.size(); ++i)
			avgDeltaCost_deltaInputs[i] /= (float)c_trainingData.size();

		#if LOG_TO_CSV_NUMSAMPLES() > 0
			const size_t trainingInterval = (c_numTrainings / (LOG_TO_CSV_NUMSAMPLES() - 1));
			if (file != nullptr && (trainingIndex % trainingInterval == 0 || trainingIndex == c_numTrainings - 1))
			{
				// log to the csv
				fprintf(file, "\"%zi\",\"%f\",\"%f\"\n", trainingIndex, avgError, avgCost);
			}
		#endif

		// adjust weights and biases
		for (size_t i = 0; i < weights.size(); ++i)
			weights[i] -= avgDeltaCost_deltaWeights[i] * c_learningRate;
		for (size_t i = 0; i < biases.size(); ++i)
			biases[i] -= avgDeltaCost_deltaBiases[i] * c_learningRate;
	}

	printf("Example4 Final Error: %f\n", avgError);

	#if LOG_TO_CSV_NUMSAMPLES() > 0
		if (file != nullptr)
			fclose(file);
	#endif
}

int main (int argc, char **argv)
{
	Example1();
	Example2();
	Example3();
	Example4();
	system("pause");
	return 0;
}


/*

Blog Notes:



Blog Notes Talked About:
 * dy/dx means: if i add one to x, how much will y change? really a ratio though. only garaunteed true for an infinitely small step, but we can take larger steps with decent results.
 * gradient vector: a vector that points in the direction that makes the function get the largest.
 * NN's are not an exact science, but more of an art form.  There exist other techniques that help make better NNs in certain situations. go learn about em!
 * mention how the gradients in O0, O1 are smaller than O2, O3 in last example.  This is called vanishing gradient, but really it can also be exploding gradient. An open research problem with deeper neural networks.
 * deltaCost/deltaZ is defined as the error of a neuron.
 * different activation functions just mean different derivatives for calculation of dO/dZ, the rest is the same for training.
 * mention cross entropy and show a graph of why it's needed. slow learning rates at beginning and end!
 ? is this multithreaded friendly? it is SIMD friendly for sure! code meant to be understandable
 * talk about deltaCost_deltaInput!
  * talk about ability to adjust input to give desired input, instead of modifying weights and biases.
  * this is how the deepvis stuff works apparently.


Not talking about:
 * why is it called backpropagation of error, when it propagates derivatives, not error?
  * dError/dOutput is (output-target), and we go from there with the chain rule, so it really is the error propagating backwards!
  * it isn't linear though, so you still need to take small steps.
 * learning rate can be adjusted. maybe start big and get small.
  * some people also have tried using "momentum" to try and avoid local minima
 * talk about how to do this with matrix based math? Or that it's possible (notice all the dot products!)
 ? compare numeric derivatives, dual numbers and back propagation for perf? or just mention which is faster than which? Also mention genetic algorithm methods
 ? report error instead of cost in all the examples? or maybe also report error?
 * visualize some neural net output with your visualizers you made, and link to em?


Great Links:
 A Step by Step Backpropagation Example
 https://mattmazur.com/2015/03/17/a-step-by-step-backpropagation-example/
 Neural networks and deep learning
 http://neuralnetworksanddeeplearning.com
 Backpropogation is Just Steepest Descent with Automatic Differentiation
 https://idontgetoutmuch.wordpress.com/2013/10/13/backpropogation-is-just-steepest-descent-with-automatic-differentiation-2/
 Chain Rule
 http://www.sosmath.com/calculus/diff/der04/der04.html
 Deepvis
 http://yosinski.com/deepvis


Future Blog posts
 1) Recipe: Feedforward mnist character recognition, w/ html5 demo.
 2) Recipe: Convolutional mnist character recognition, w/ html5 demo.
 3) Recurrent network post and demo?

*/