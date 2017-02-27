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

	//TODO: did you calculate error correctly? you have derivatives but where is the error itself?!


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

int main (int argc, char **argv)
{
	Testv1();
	Testv2();
	Testv3();
	system("pause");
}

/*

TODO:
4) make a network that has multiple neurons in multiple layers. maybe combine with below for multiple outputs?
5) make a network with multiple outputs
 * maybe make this network and verify values? https://mattmazur.com/2015/03/17/a-step-by-step-backpropagation-example/
6) cross correlation (or whatever) cost function that trains faster (or make this 3)
 * or save this for recipe post?
7) matrix form of this junk?
8) do something where you adjust input via gradient descent to get the desired output.  A way of asking it "what would it take to make a 1?"

* dunno eventual goal of this program yet, or specific post details.
 ? multithreaded?
 ? matrix based math?
 ? compare backprop to dual numbers and numeric derivatives?
 ? output csvs to make graphs and such?
 ? support / compare different activation functions?
 ? why does averaging cost and deltas work?
 ? dropout etc?
 ? show perf timing?
 ? adjust weights during backwards phase, instead of storing deltas and doing it separately? actually no since you need to batch it!
 * maybe need to rename cost to half mean squared error or something?
 * find todos

Great Links:
 A Step by Step Backpropagation Example
 https://mattmazur.com/2015/03/17/a-step-by-step-backpropagation-example/
 Neural networks and deep learning
 http://neuralnetworksanddeeplearning.com
 Backpropogation is Just Steepest Descent with Automatic Differentiation
 https://idontgetoutmuch.wordpress.com/2013/10/13/backpropogation-is-just-steepest-descent-with-automatic-differentiation-2/

Blog Notes:
 * dy/dx means: if i add one to x, how much will y change? really a ratio though. only garaunteed true for an infinitely small step, but we can take larger steps with decent results.
 * gradient vector: a vector that points in the direction that makes the function get the largest.
 * deltaCost/deltaZ is defined as the error of a neuron.
 * visualize some neural net output with your visualizers you made, and link to em!

Blog:
 1) maybe backpropagation as first post?
  * could compare numeric derivatives, dual numbers, back propagation for perf etc.
 2) could do a feedforward recipe next for mnist character recognition, w/ html5 demo.
 3) then convolutional network post and demo?
 4) then recurrent network post and demo?
 ? should we do something where we adjust input to match output? maybe in 2,3,4?

===== TEST V1 =====
Start out by making a simple network and train it. (done)
 * input neuron, single output neuron, one input and output to learn.

--v1a--
in 0, out 1
 * 410000 trainings
starting: 
 * weight: 0.3
 * bias: 0.5
ending: 
 * weight: 0.3
 * bias: 5.3

--v1b--
in 1, out 0
 * 205000 trainings
starting:
 * weight: 0.3
 * bias: 0.5
ending:
 * weight: -2.75
 * bias: -2.55

===== TEST V2 =====
Make a network that fits multiple data points
 * not gate

in 0, out 1
in 1, out 0
 * 530000 trainings
starting:
 * weight: 0.3
 * bias: 0.5
ending:
 * weight: -9.21
 * bias: 4.50

===== TEST V3 =====
Make a network with two neurons (input and output neuron) that fits multiple data points
 * not gate again

in 0, out 1
in 1, out 0
 * 550000 trainings
starting:
 * weights: 0.3, 0.1
 * biases: 0.2, 0.6
ending:
 * weights: 6.56, -9.93
 * biases: -3.26, 4.88

*/