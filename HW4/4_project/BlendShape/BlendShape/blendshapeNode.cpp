#include "blendshapeNode.h"
#include <string>

// For local testing of nodes you can use any identifier between
// 0x00000000 and 0x0007ffff, but for any node that you plan to use for
// more permanent purposes, you should get a universally unique id from Autodesk Support. 
// You will be assigned a unique range that you can manage on your own.
MTypeId     BlendMesh::id(0x00000232);

// Attributes
MObject     BlendMesh::aBlendMesh;
MObject     BlendMesh::aBlendWeight;


BlendMesh::BlendMesh()
{
}


BlendMesh::~BlendMesh()
{
}


void* BlendMesh::creator()
{
	return new BlendMesh();
}


MStatus BlendMesh::deform(MDataBlock& data, MItGeometry& itGeo,
	const MMatrix& localToWorldMatrix, unsigned int geomIndex)
{
	MStatus status;

	float env = data.inputValue(envelope).asFloat();
	if (env == 0.0) return MS::kSuccess;

	MArrayDataHandle ArrayBlendMeshs = data.inputArrayValue(aBlendMesh, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MArrayDataHandle ArrayBlendWeight = data.inputArrayValue(aBlendWeight, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	unsigned int numSourceMesh = ArrayBlendMeshs.elementCount();

	std::vector<MPointArray> arrayBlendPoints; //source meshes
	std::vector<float> arrayWeights; //source weights
	for (unsigned int i = 0; i<numSourceMesh; i++)
	{

		// ------------------------------------------------------------------------------ //
		//									  TO DO - (2)							  
		// ------------------------------------------------------------------------------ //
		// 1. Get the mesh and weight of i-th blendshape(source mesh)			  
		// 2. Push the vertex points of the mesh to variable 'arrayBlendPoints'	  
		// 3. Push the weight to variable 'arrayWeights'						  
		// ------------------------------------------------------------------------------ //

		// Get the mesh and weight of i-th blendshape
		ArrayBlendMeshs.jumpToElement(i);
		ArrayBlendWeight.jumpToElement(i);

		MDataHandle meshHandle = ArrayBlendMeshs.inputValue(&status);
		MDataHandle weightHandle = ArrayBlendWeight.inputValue(&status);

		// Push the vertex points of the mesh to the variable 'arrayBlendPoints'
		MPointArray tmpPointArray;
		MFnMesh fnMesh(meshHandle.asMesh());
		fnMesh.getPoints(tmpPointArray, MSpace::kObject);
		arrayBlendPoints.push_back(tmpPointArray);

		// Push the weight to the variable 'arrayWeight'
		arrayWeights.push_back(weightHandle.asFloat());
	}

	MPoint point;
	float w;

	for (itGeo.reset(); !itGeo.isDone(); itGeo.next())
	{
		point = itGeo.position();
		w = weightValue(data, geomIndex, itGeo.index()); // About this 'w' (and 'envelope'), please read the book Ch4.8(Deformers). It's different with the blendshape weight:)

		// -------------------------------------------------------------------------------- //
		//									    TO DO - (3)										
		// -------------------------------------------------------------------------------- //
		// 1. Iterate over the source meshes
		// 2. Calculate the target point using the blendshape equation (HW#4 PPT 12p)		
		// -------------------------------------------------------------------------------- //
		
		MPoint targetPoint = point;

		// Iterate over all source meshes
		for (unsigned int i = 0; i < numSourceMesh; i++) {
			// Calculate the target point using the blendshape equation
			// New face = neutral face + ( sum(deformed face - neutral face) * weight )
			MPoint delta = (arrayBlendPoints[i][itGeo.index()] - point) * arrayWeights[i] * env * w;
			targetPoint += delta;
		}

		itGeo.setPosition(targetPoint);
	}


	return MS::kSuccess;
}


MStatus BlendMesh::initialize()
{
	MFnTypedAttribute tAttr;
	MFnNumericAttribute nAttr;

	// Create attribute for source meshes
	aBlendMesh = tAttr.create("blendMesh", "bM", MFnData::kMesh);
	tAttr.setArray(true);
	tAttr.setUsesArrayDataBuilder(true);
	addAttribute(aBlendMesh);
	attributeAffects(aBlendMesh, outputGeom);


	// --------------------------------------------------------------------------------- //
	//										  TO DO - (1)							  
	// --------------------------------------------------------------------------------- //
	// 1. Create attribute for weights 
	// * Set the range of weight from 0.0 to 1.0 (float)
	// --------------------------------------------------------------------------------- //

	// Create attribute for weights
	aBlendWeight = nAttr.create("blendWeight", "bW", MFnNumericData::kFloat, 0.0);
	nAttr.setArray(true);
	nAttr.setKeyable(true);
	nAttr.setUsesArrayDataBuilder(true);
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);
	addAttribute(aBlendWeight);
	attributeAffects(aBlendWeight, outputGeom);
	   	  	
	return MS::kSuccess;
}
