#include <stdio.h>
#include <array>

// Nonzero value enables csv logging.
#define LOG_TO_CSV_NUMSAMPLES() 50

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
	const size_t c_numTrainings = 400000;

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

int main (int argc, char **argv)
{
	Example1();
	Example2();
	system("pause");
	return 0;
}


/*

Code Notes:
 * Need to make new samples. The above code is just for me!!
 ! one of the samples could be the one talked about earlier in the post?
 * What samples should we make?
  * probably ones that tie into the blog post. start simple go more complex.
   1) single neuron. learn one input/output
   2) single neuron. multiple inputs/outputs
   3) single neuron per layer, two layers. multiple inputs/outputs
   4) multiple neurons per layer, two layers. multiple inputs/outputs
 * we should play with learning rates, and have it make a csv that can show error graphed over time. maybe also show weights and biases and activations and things?
 * make something that changes input via gradient descent to get a desired output

Blog Notes:
 * talk about deltaError_deltaInput!
 * deltaCost/deltaZ is defined as the error of a neuron.
 * visualize some neural net output with your visualizers you made, and link to em!
 * why is it called backpropagation of error, when it propagates derivatives, not error?
  * dError/dOutput is (output-target), and we go from there with the chain rule, so it really is the error propagating backwards!
  * it isn't linear though, so you still need to take small steps.
 * learning rate can be adjusted. maybe start big and get small.
  * some people also have tried using "momentum" to try and avoid local minima
 * talk about how to do this with matrix based math? Or that it's possible (notice all the dot products!)
 * different activation functions just mean different derivatives for calculation of dO/dZ, the rest is the same for training.
 ? is this multithreaded friendly? it is SIMD friendly for sure! code meant to be understandable
 ? compare numeric derivatives, dual numbers and back propagation for perf? or just mention which is faster than which? Also mention genetic algorithm methods


Blog Notes Talked About:
* dy/dx means: if i add one to x, how much will y change? really a ratio though. only garaunteed true for an infinitely small step, but we can take larger steps with decent results.
* gradient vector: a vector that points in the direction that makes the function get the largest.
* NN's are not an exact science, but more of an art form.  There exist other techniques that help make better NNs in certain situations. go learn about em!


Great Links:
 A Step by Step Backpropagation Example
 https://mattmazur.com/2015/03/17/a-step-by-step-backpropagation-example/
 Neural networks and deep learning
 http://neuralnetworksanddeeplearning.com
 Backpropogation is Just Steepest Descent with Automatic Differentiation
 https://idontgetoutmuch.wordpress.com/2013/10/13/backpropogation-is-just-steepest-descent-with-automatic-differentiation-2/
 Chain Rule
 http://www.sosmath.com/calculus/diff/der04/der04.html

Future Blog posts
 1) Recipe: Feedforward mnist character recognition, w/ html5 demo.
 2) Recipe: Convolutional mnist character recognition, w/ html5 demo.
 3) Recurrent network post and demo?

*/