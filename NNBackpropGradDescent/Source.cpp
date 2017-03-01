#include <stdio.h>
#include <cmath>
#include <vector>
#include <array>

//====================================== V1 ======================================
// One neuron learning one input mapping to one output

void RunNNv1 (float inInput, float inWeight, float inBias, float inTarget, float& outCost, float& outDeltaInput, float& outDeltaWeight, float& outDeltaBias, float &outOutput)
{
	// calculate Z (weighted input) and O (activation function of weighted input)
	float Z = inInput * inWeight + inBias;
	float O = 1.0f / (1.0f + std::exp(-Z));
	
	// calculate half squared error
	outCost = 0.5f * (inTarget - O) * (inTarget - O);

	// dCost/dO = O - target
	float deltaCost_deltaO = O - inTarget;

	// dO/dZ = O * (1 - O)
	float deltaO_deltaZ = O * (1.0f - O);

	// dZ/dWeight = input
	float deltaZ_deltaWeight = inInput;

	// dZ/dBias = 1
	const float deltaZ_deltaBias = 1.0f;

	// dZ/dInput = weight
	float deltaZ_deltaInput = inWeight;



	// dCost/dWeight = dCost/dO * dO/dZ * dZ/dWeight
	outDeltaWeight = deltaCost_deltaO * deltaO_deltaZ * deltaZ_deltaWeight;

	// dCost/dBias = dCost/dO * dO/dZ * dZ/dBias
	outDeltaBias = deltaCost_deltaO * deltaO_deltaZ * deltaZ_deltaBias;

	// dCost/dInput = dCost/dO * dO/dZ * dZ/dInput
	outDeltaInput = deltaCost_deltaO * deltaO_deltaZ * deltaZ_deltaInput;

	// return the output of the network
	outOutput = O;
}

void Testv1 (void)
{
	const float input = 1.0f;
	const float target = 0.0f;

	const float c_learningRate = 0.05f;

	float weight = 0.3f;
	float bias = 0.5f;

	float cost = 0.0f;
	float deltaInput = 0.0f;
	float deltaWeight = 0.0f;
	float deltaBias = 0.0f;
	float output = 0.0f;

	const int c_numTrainings = 205000;
	const int c_reportInterval = c_numTrainings / 20;
	for (int i = 0; i < c_numTrainings; ++i)
	{
		RunNNv1(input, weight, bias, target, cost, deltaInput, deltaWeight, deltaBias, output);

		if (i % c_reportInterval == 0 || i == c_numTrainings - 1)
		{
			printf("[%i] cost: %0.4f (%0.2f vs %0.2f)\n", i, cost, output, target);
		}

		weight -= deltaWeight * c_learningRate;
		bias -= deltaBias * c_learningRate;
	}

	printf("\nweight=%0.2f\nbias=%0.2f\n\n", weight, bias);
}

//====================================== V2 ======================================
// One neuron learning two input to output mappings

void RunNNv2 (float inInput, float inWeight, float inBias, float inTarget, float& outCost, float& outDeltaInput, float& outDeltaWeight, float& outDeltaBias)
{
	// calculate Z (weighted input) and O (activation function of weighted input)
	float Z = inInput * inWeight + inBias;
	float O = 1.0f / (1.0f + std::exp(-Z));
	
	// calculate half squared error
	outCost = 0.5f * (inTarget - O) * (inTarget - O);

	// dCost/dO = O - target
	float deltaCost_deltaO = O - inTarget;

	// dO/dZ = O * (1 - O)
	float deltaO_deltaZ = O * (1.0f - O);

	// dZ/dWeight = input
	float deltaZ_deltaWeight = inInput;

	// dZ/dBias = 1
	const float deltaZ_deltaBias = 1.0f;

	// dZ/dInput = weight
	float deltaZ_deltaInput = inWeight;



	// dCost/dWeight = dCost/dO * dO/dZ * dZ/dWeight
	outDeltaWeight = deltaCost_deltaO * deltaO_deltaZ * deltaZ_deltaWeight;

	// dCost/dBias = dCost/dO * dO/dZ * dZ/dBias
	outDeltaBias = deltaCost_deltaO * deltaO_deltaZ * deltaZ_deltaBias;

	// dCost/dInput = dCost/dO * dO/dZ * dZ/dInput
	outDeltaInput = deltaCost_deltaO * deltaO_deltaZ * deltaZ_deltaInput;
}

void Testv2 (void)
{
	std::vector<std::array<float, 2>> trainings = {
		{1.0f, 0.0f},
		{0.0f, 1.0f}
	};

	const float c_learningRate = 0.05f;

	float weight = 0.3f;
	float bias = 0.5f;

	const int c_numTrainings = 530000;
	const int c_reportInterval = c_numTrainings / 20;
	for (int i = 0; i < c_numTrainings; ++i)
	{
		float avgCost = 0.0f;
		float avgDeltaInput = 0.0f;
		float avgDeltaWeight = 0.0f;
		float avgDeltaBias = 0.0f;

		const size_t trainingCount = trainings.size();
		for (size_t trainingIndex = 0; trainingIndex < trainingCount; ++trainingIndex)
		{
			float cost = 0.0f;
			float deltaInput = 0.0f;
			float deltaWeight = 0.0f;
			float deltaBias = 0.0f;

			RunNNv2(trainings[trainingIndex][0], weight, bias, trainings[trainingIndex][1], cost, deltaInput, deltaWeight, deltaBias);

			avgCost += cost;
			avgDeltaInput += deltaInput;
			avgDeltaWeight += deltaWeight;
			avgDeltaBias += deltaBias;
		}

		avgCost /= (float)trainingCount;
		avgDeltaInput /= (float)trainingCount;
		avgDeltaWeight /= (float)trainingCount;
		avgDeltaBias /= (float)trainingCount;
		
		if (i % c_reportInterval == 0 || i == c_numTrainings - 1)
		{
			printf("[%i] cost: %0.4f\n", i, avgCost);
		}

		weight -= avgDeltaWeight * c_learningRate;
		bias -= avgDeltaBias * c_learningRate;
	}

	printf("\nweight=%0.2f\nbias=%0.2f\n\n", weight, bias);
}

//====================================== V3 ======================================
// two neurons in series, learning two input to output mappings

void RunNNv3 (float inInput, const std::array<float,2>& inWeights, const std::array<float, 2> inBiases, float inTarget, float& outCost, float& outDeltaInput, std::array<float, 2>& outDeltaWeights, std::array<float, 2>& outDeltaBiases)
{

	// ----- Forward Pass -----

	// calculate Z0 (weighted input) and O0 (activation function of weighted input) for first neuron
	float Z0 = inInput * inWeights[0] + inBiases[0];
	float O0 = 1.0f / (1.0f + std::exp(-Z0));

	// calculate Z1 (weighted input) and O1 (activation function of weighted input) for second neuron
	float Z1 = O0 * inWeights[1] + inBiases[1];
	float O1 = 1.0f / (1.0f + std::exp(-Z1));

	// calculate half squared error
	outCost = 0.5f * (inTarget - O1) * (inTarget - O1);



	// ----- Backward Pass : Output Neuron. Neuron 1 -----

	// dCost/dO1 = O1 - target
	float deltaCost_deltaO1 = O1 - inTarget;
	
	// dO1/dZ1 = O1 * (1 - O1)
	float deltaO1_deltaZ1 = O1 * (1.0f - O1);

	// dZ1/dWeight1 = O0
	float deltaZ1_deltaWeight1 = O0;

	// the error in neuron 1 is defined as dCost/dZ1
	// dCost/dZ1 = dCost/dO1 * dO1/dZ1
	float errorNeuron1 = deltaCost_deltaO1 * deltaO1_deltaZ1;

	// dCost/dBias1 = dCost/dZ1 = error in neuron 1
	float deltaCost_deltaBias1 = errorNeuron1;

	// dCost/dWeight1 = dCost/dZ1 * dZ1/dWeight1 = errorNeuron1 * dZ1/dWeight1;
	float deltaCost_deltaWeight1 = errorNeuron1 * deltaZ1_deltaWeight1;



	// ----- Backward Pass : Input Neuron. Neuron 0 -----

	// dZ1/dO0 = Weight1
	float deltaZ1_deltaO0 = inWeights[1];

	// dO0/dZ0 = O0 * (1 - O0)
	float deltaO0_deltaZ0 = O0 * (1.0f - O0);

	// dZ0/dWeight0 = input
	float deltaZ0_deltaWeight0 = inInput;

	// dZ0/dInput = weight0
	float deltaZ0_deltaInput = inWeights[0];

	// the error in neuron 0 is defined as dCost/dZ0
	// dCost/dZ0 = dCost/dO1 * dO1/dZ1 * dZ1/dO0 * dO0/dZ0 = errorNeuron1 * dZ1/dO0 * dO0/dZ0
	float errorNeuron0 = errorNeuron1 * deltaZ1_deltaO0 * deltaO0_deltaZ0;



	// dCost/dBias0 = dCost/dZ0 = errorNeuron0
	float deltaCost_deltaBias0 = errorNeuron0;

	// dCost/dWeight0 = dCost/dZ0 * dZ0/dWeight0 = errorNeuron0 * dZ0/dWeight0
	float deltaCost_deltaWeight0 = errorNeuron0 * deltaZ0_deltaWeight0;

	// dCost/dInput = dCost/dZ0 * dZ0/dInput = errorNeuron0 * dZ0/dInput
	float deltaCost_deltaInput = errorNeuron0 * deltaZ0_deltaInput;



	// ----- Set outputs -----

	outDeltaWeights[0] = deltaCost_deltaWeight0;
	outDeltaWeights[1] = deltaCost_deltaWeight1;
	outDeltaBiases[0] = deltaCost_deltaBias0;
	outDeltaBiases[1] = deltaCost_deltaBias1;
	outDeltaInput = deltaCost_deltaInput;
}

void Testv3 (void)
{

	std::vector<std::array<float, 2>> trainings = {
		{1.0f, 0.0f},
		{0.0f, 1.0f}
	};

	const float c_learningRate = 0.05f;

	std::array<float, 2> weights = { 0.3f, 0.1f };
	std::array<float, 2> biases =  { 0.2f, 0.6f };

	const int c_numTrainings = 550000;
	const int c_reportInterval = c_numTrainings / 20;
	for (int i = 0; i < c_numTrainings; ++i)
	{
		float avgCost = 0.0f;
		float avgDeltaInput = 0.0f;
		std::array<float, 2> avgDeltaWeights = { 0.0f, 0.0f };
		std::array<float, 2> avgDeltaBiases = { 0.0f, 0.0f };

		const size_t trainingCount = trainings.size();
		for (size_t trainingIndex = 0; trainingIndex < trainingCount; ++trainingIndex)
		{
			float cost = 0.0f;
			float deltaInput = 0.0f;
			std::array<float, 2> deltaWeights = { 0.0f, 0.0f };
			std::array<float, 2> deltaBiases = { 0.0f, 0.0f };

			RunNNv3(trainings[trainingIndex][0], weights, biases, trainings[trainingIndex][1], cost, deltaInput, deltaWeights, deltaBiases);

			avgCost += cost;
			avgDeltaInput += deltaInput;
			avgDeltaWeights[0] += deltaWeights[0];
			avgDeltaWeights[1] += deltaWeights[1];
			avgDeltaBiases[0] += deltaBiases[0];
			avgDeltaBiases[1] += deltaBiases[1];
		}

		avgCost /= (float)trainingCount;
		avgDeltaInput /= (float)trainingCount;
		avgDeltaWeights[0] /= (float)trainingCount;
		avgDeltaWeights[1] /= (float)trainingCount;
		avgDeltaBiases[0] /= (float)trainingCount;
		avgDeltaBiases[1] /= (float)trainingCount;
		
		if (i % c_reportInterval == 0 || i == c_numTrainings - 1)
		{
			printf("[%i] cost: %0.4f\n", i, avgCost);
		}

		weights[0] -= avgDeltaWeights[0] * c_learningRate;
		weights[1] -= avgDeltaWeights[1] * c_learningRate;
		biases[0] -= avgDeltaBiases[0] * c_learningRate;
		biases[1] -= avgDeltaBiases[1] * c_learningRate;
	}

	printf("\nweights=[%0.2f, %0.2f]\nbiases=[%0.2f, %0.2f]\n\n", weights[0], weights[1], biases[0], biases[1]);
}

//====================================== V4 ======================================
/*
  One input layer, one hidden layer, one output layer, all with two neurons each.
  Learning one input to output mapping.

  I0---N0---N2--->
    \ /  \ /
     X    X
	/ \  / \
  I1---N1---N3--->

  W0 = I0->N0
  W1 = I1->N0
  W2 = I0->N1
  W3 = I1->N1

  W4 = N1->N2
  W5 = N2->N2
  W6 = N1->N3
  W7 = N2->N3

  B0 = N0
  B1 = N1
  B2 = N2
  B3 = N3

*/

void RunNNv4 (const std::array<float,2>& inInput, const std::array<float, 8>& inWeights, const std::array<float, 4> inBiases, const std::array<float, 2>& inTarget, float& outCost, std::array<float,2>& outDeltaInput, std::array<float, 8>& outDeltaWeights, std::array<float, 4>& outDeltaBiases)
{

	// ----- Forward Pass -----
	
	// calculate Z0,Z1 (weighted input) and O0,O1 (activation function of weighted input) for hidden layer of neurons
	float Z0 = inInput[0] * inWeights[0] + inInput[1] * inWeights[1] + inBiases[0];
	float Z1 = inInput[0] * inWeights[2] + inInput[1] * inWeights[3] + inBiases[1];
	float O0 = 1.0f / (1.0f + std::exp(-Z0));
	float O1 = 1.0f / (1.0f + std::exp(-Z1));

	// Calculate Z2,Z3 (weighted input) and O2,O3 (activation function of weighted input) for output layer of neurons
	float Z2 = O0 * inWeights[4] + O1 * inWeights[5] + inBiases[2];
	float Z3 = O0 * inWeights[6] + O1 * inWeights[7] + inBiases[3];
	float O2 = 1.0f / (1.0f + std::exp(-Z2));
	float O3 = 1.0f / (1.0f + std::exp(-Z3));

	// calculate half squared error
	float diff0 = inTarget[0] - O2;
	float diff1 = inTarget[1] - O3;
	outCost = 0.5f * (diff0 * diff0 + diff1 * diff1);


	// ----- Backward Pass : Output Neuron Layer -----

	// dCost/dO2 = O2 - target0
	// dCost/dO3 = O3 - target1
	float deltaCost_deltaO2 = O2 - inTarget[0];
	float deltaCost_deltaO3 = O3 - inTarget[1];

	// N2Error = dCost/dZ2 = dCost/dO2 * dO2/dZ2 = deltaCost_deltaO2 * O2 * (1 - O2)
	// N3Error = dCost/dZ3 = dCost/dO3 * dO3/dZ3 = deltaCost_deltaO3 * O3 * (1 - O3)
	float N2Error = deltaCost_deltaO2 * O2 * (1.0f - O2);
	float N3Error = deltaCost_deltaO3 * O3 * (1.0f - O3);

	// dCost/dWeight4 = dCost/dZ2 * dZ2/dWeight4 = N2Error * O0
	// dCost/dWeight5 = dCost/dZ2 * dZ2/dWeight5 = N2Error * O1
	float deltaCost_deltaWeight4 = N2Error * O0;
	float deltaCost_deltaWeight5 = N2Error * O1;

	// dCost/dWeight6 = dCost/dZ3 * dZ3/dWeight6 = N3Error * O0
	// dCost/dWeight7 = dCost/dZ3 * dZ3/dWeight7 = N3Error * O1
	float deltaCost_deltaWeight6 = N3Error * O0;
	float deltaCost_deltaWeight7 = N3Error * O1;


	// ----- Backward Pass : Output Neuron Layer -----

	// dCost/dO0 = (dCost/dZ2 * dZ2/dO0) + (dCost/dZ3 * dZ3/dO0) = N2Error * Weight4 + N3Error * Weight6
	// dCost/dO1 = (dCost/dZ2 * dZ2/dO1) + (dCost/dZ3 * dZ3/dO1) = N2Error * Weight5 + N3Error * Weight7
	float deltaCost_deltaO0 = (N2Error * inWeights[4]) + (N3Error * inWeights[6]);
	float deltaCost_deltaO1 = (N2Error * inWeights[5]) + (N3Error * inWeights[7]);

	// N0Error = dCost/dZ0 = dCost/dO0 * dO0/dZ0 = deltaCost_deltaO0 * O0 * (1 - O0)
	// N1Error = dCost/dZ1 = dCost/dO1 * dO1/dZ1 = deltaCost_deltaO1 * O1 * (1 - O1)
	float N0Error = deltaCost_deltaO0 * O0 * (1.0f - O0);
	float N1Error = deltaCost_deltaO1 * O1 * (1.0f - O1);

	// dCost/dWeight0 = dCost/dZ0 * dZ0/dWeight0 = N0Error * input0
	// dCost/dWeight1 = dCost/dZ0 * dZ0/dWeight1 = N0Error * input1
	float deltaCost_deltaWeight0 = N0Error * inInput[0];
	float deltaCost_deltaWeight1 = N0Error * inInput[1];
	
	// dCost/dWeight2 = dCost/dZ1 * dZ1/dWeight0 = N1Error * input0
	// dCost/dWeight3 = dCost/dZ1 * dZ1/dWeight1 = N1Error * input1
	float deltaCost_deltaWeight2 = N1Error * inInput[0];
	float deltaCost_deltaWeight3 = N1Error * inInput[1];
	

	// ----- Backward Pass : Input Layer -----

	// dCost/dInput0 = (dCost/dZ0 * dZ0/dInput0) + (dCost/dZ1 * dZ1/dInput0) = N0Error * Weight0 + N1Error * Weight2
	// dCost/dInput1 = (dCost/dZ0 * dZ0/dInput1) + (dCost/dZ1 * dZ1/dInput1) = N0Error * Weight1 + N1Error * Weight3
	float deltaCost_deltaInput0 = N0Error * inWeights[0] + N1Error * inWeights[2];
	float deltaCost_deltaInput1 = N0Error * inWeights[1] + N1Error * inWeights[3];
		

	// ----- Set outputs -----
	outDeltaWeights[0] = deltaCost_deltaWeight0;
	outDeltaWeights[1] = deltaCost_deltaWeight1;
	outDeltaWeights[2] = deltaCost_deltaWeight2;
	outDeltaWeights[3] = deltaCost_deltaWeight3;
	outDeltaWeights[4] = deltaCost_deltaWeight4;
	outDeltaWeights[5] = deltaCost_deltaWeight5;
	outDeltaWeights[6] = deltaCost_deltaWeight6;
	outDeltaWeights[7] = deltaCost_deltaWeight7;

	outDeltaBiases[0] = N0Error;
	outDeltaBiases[1] = N1Error;
	outDeltaBiases[2] = N2Error;
	outDeltaBiases[3] = N3Error;

	outDeltaInput[0] = deltaCost_deltaInput0;
	outDeltaInput[1] = deltaCost_deltaInput1;
}

void Testv4 (void)
{
	// matching the network from here: https://mattmazur.com/2015/03/17/a-step-by-step-backpropagation-example/
	std::array<float, 2> input = { 0.05f, 0.1f };
	std::array<float, 2> output = { 0.01f, 0.99f };

	std::array<float, 8> weights = {0.15f, 0.20f, 0.25f, 0.30f, 0.40f, 0.45f, 0.50f, 0.55f};
	std::array<float, 4> biases = {0.35f, 0.35f, 0.60f, 0.60f};

	const float c_learningRate = 0.5f;

	const int c_numTrainings = 3000;
	const int c_reportInterval = c_numTrainings / 20;
	for (int i = 0; i < c_numTrainings; ++i)
	{
		float cost = 0.0f;
		std::array<float, 2> deltaInput = { 0.0f, 0.0f };
		std::array<float, 8> deltaWeights = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		std::array<float, 4> deltaBiases = { 0.0f, 0.0f, 0.0f, 0.0f };

		RunNNv4(input, weights, biases, output, cost, deltaInput, deltaWeights, deltaBiases);

		if (i % c_reportInterval == 0 || i == c_numTrainings - 1)
		{
			printf("[%i] cost: %0.4f\n", i, cost);
		}

		for (size_t w = 0, wc = weights.size(); w < wc; ++w)
			weights[w] -= deltaWeights[w] * c_learningRate;

		for (size_t b = 0, bc = biases.size(); b < bc; ++b)
			biases[b] -= deltaBiases[b] * c_learningRate;
	}

	printf("\nweights=[");
	for (size_t i = 0; i < weights.size(); ++i)
	{
		if (i > 0)
			printf(", ");
		printf("%0.2f", weights[i]);
	}
	printf("]\nbiases=[");
	for (size_t i = 0; i < biases.size(); ++i)
	{
		if (i > 0)
			printf(", ");
		printf("%0.2f", biases[i]);
	}
	printf("]\n\n");
}

int main (int argc, char **argv)
{
	//Testv1();
	//Testv2();
	//Testv3();
	Testv4();
	system("pause");
}

/*

Code Notes:
 * Need to make new samples. The above code is just for me!!
 * What samples should we make?
  * probably ones that tie into the blog post. start simple go more complex.
   1) single neuron. learn one input/output
   2) single neuron. multiple inputs/outputs
   3) single neuron per layer, two layers. multiple inputs/outputs
   4) multiple neurons per layer, two layers. multiple inputs/outputs
 * we should play with learning rates, and have it make a csv that can show error graphed over time. maybe also show weights and biases and activations and things?
 * make something that changes input via gradient descent to get a desired output

Blog Notes:
 * dy/dx means: if i add one to x, how much will y change? really a ratio though. only garaunteed true for an infinitely small step, but we can take larger steps with decent results.
 * gradient vector: a vector that points in the direction that makes the function get the largest.
 * deltaCost/deltaZ is defined as the error of a neuron.
 * visualize some neural net output with your visualizers you made, and link to em!
 * why is it called backpropagation of error, when it propagates derivatives, not error?
  * dError/dOutput is (output-target), and we go from there with the chain rule, so it really is the error propagating backwards!
  * it isn't linear though, so you still need to take small steps.
 * learning rate can be adjusted. maybe start big and get small.
 * some people also have tried using "momentum" to try and avoid local minima
 * talk about how to do this with matrix based math?
 * different activation functions just mean different derivatives for calculation of dO/dZ, the rest is the same for training.
 ? is this multithreaded friendly? it is SIMD friendly for sure!
 ? compare numeric derivatives, dual numbers and back propagation for perf? or just mention which is faster than which?

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