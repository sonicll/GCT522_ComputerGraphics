//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+

//#include <math.h>
//
#include <maya/MPxNode.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnLightDataAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MFnPlugin.h>
#include <maya/MDrawRegistry.h>
#include <maya/MGlobal.h>

// add for raytracing api enhancement
#include <maya/MRenderUtil.h>

#include "toonShaderOverride.h"

////////////////////////////////////////////////////////////////////////
//
// DESCRIPTION:
// 
// Produces dependency graph node ToonNode

// This node is an example of a Phong shader and how to build a dependency node as a surface shader in Maya.

// The inputs for this node can be found in the Maya UI on the Attribute Editor for the node.

// The output attribute of the node is called "outColor". It is a 3 float value that represents the resulting color produced by the node. 
// To use this shader, create a phongNode with Shading Group or connect its output to a Shading Group��s "SurfaceShader" attribute. 
//
///////////////////////////////////////////////////////

class ToonNode : public MPxNode
{
public:
	ToonNode();
	virtual           ~ToonNode();

	virtual MStatus   compute(const MPlug&, MDataBlock&);
	virtual void      postConstructor();

	static void *     creator();
	static MStatus    initialize();
	static MTypeId    id;

private:
	static MObject aColor;
	static MObject aTranslucenceCoeff;
	static MObject aDiffuseReflectivity;
	static MObject aIncandescence;
	static MObject aPointCamera;
	static MObject aNormalCamera;
	static MObject aLightDirection;
	static MObject aLightIntensity;
	static MObject aPower;
	static MObject aSpecularity;
	static MObject aLightAmbient;
	static MObject aLightDiffuse;
	static MObject aLightSpecular;
	static MObject aLightShadowFraction;
	static MObject aPreShadowIntensity;
	static MObject aLightBlindData;
	static MObject aLightData;

	static MObject aRayOrigin;
	static MObject aRayDirection;

	static MObject aObjectId;
	static MObject aRaySampler;
	static MObject aRayDepth;

	static MObject aReflectGain;

	static MObject aTriangleNormalCamera;

	static MObject aOutColor;

};

// Static data
MTypeId ToonNode::id(0x81001);

// Attributes
MObject ToonNode::aColor;
MObject ToonNode::aTranslucenceCoeff;
MObject ToonNode::aDiffuseReflectivity;
MObject ToonNode::aIncandescence;
MObject ToonNode::aOutColor;
MObject ToonNode::aPointCamera;
MObject ToonNode::aNormalCamera;
MObject ToonNode::aLightData;
MObject ToonNode::aLightDirection;
MObject ToonNode::aLightIntensity;
MObject ToonNode::aLightAmbient;
MObject ToonNode::aLightDiffuse;
MObject ToonNode::aLightSpecular;
MObject ToonNode::aLightShadowFraction;
MObject ToonNode::aPreShadowIntensity;
MObject ToonNode::aLightBlindData;
MObject ToonNode::aPower;
MObject ToonNode::aSpecularity;

MObject ToonNode::aRayOrigin;
MObject ToonNode::aRayDirection;
MObject ToonNode::aObjectId;
MObject ToonNode::aRaySampler;
MObject ToonNode::aRayDepth;

MObject ToonNode::aReflectGain;

MObject ToonNode::aTriangleNormalCamera;

#define MAKE_INPUT(attr)						\
    CHECK_MSTATUS ( attr.setKeyable(true) );  	\
	CHECK_MSTATUS ( attr.setStorable(true) );	\
    CHECK_MSTATUS ( attr.setReadable(true) );  \
	CHECK_MSTATUS ( attr.setWritable(true) );

#define MAKE_OUTPUT(attr)							\
    CHECK_MSTATUS ( attr.setKeyable(false) ) ;  	\
	CHECK_MSTATUS ( attr.setStorable(false) );		\
    CHECK_MSTATUS ( attr.setReadable(true) ) ;  	\
	CHECK_MSTATUS ( attr.setWritable(false) );

//
// DESCRIPTION:
///////////////////////////////////////////////////////
void ToonNode::postConstructor()
{
	setMPSafe(true);
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
ToonNode::ToonNode()
{
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
ToonNode::~ToonNode()
{
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
void * ToonNode::creator()
{
	return new ToonNode();
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus ToonNode::initialize()
{
	MFnNumericAttribute nAttr;
	MFnLightDataAttribute lAttr;

	aTranslucenceCoeff = nAttr.create("translucenceCoeff", "tc",
		MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);

	aDiffuseReflectivity = nAttr.create("diffuseReflectivity", "drfl",
		MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setDefault(0.8f));

	aColor = nAttr.createColor("color", "c");
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setDefault(0.0f, 0.58824f, 0.644f));

	aIncandescence = nAttr.createColor("incandescence", "ic");
	MAKE_INPUT(nAttr);

	aOutColor = nAttr.createColor("outColor", "oc");
	MAKE_OUTPUT(nAttr);

	aPointCamera = nAttr.createPoint("pointCamera", "pc");
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setDefault(1.0f, 1.0f, 1.0f));
	CHECK_MSTATUS(nAttr.setHidden(true));

	aPower = nAttr.create("power", "pow", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setMin(0.0f));
	CHECK_MSTATUS(nAttr.setMax(200.0f));
	CHECK_MSTATUS(nAttr.setDefault(10.0f));

	aSpecularity = nAttr.create("specularity", "spc", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setMin(0.0f));
	CHECK_MSTATUS(nAttr.setMax(1.0f));
	CHECK_MSTATUS(nAttr.setDefault(0.5f));

	aReflectGain = nAttr.create("reflectionGain", "rg", MFnNumericData::kFloat);
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setMin(0.0f));
	CHECK_MSTATUS(nAttr.setMax(1.0f));
	CHECK_MSTATUS(nAttr.setDefault(0.5f));

	aNormalCamera = nAttr.createPoint("normalCamera", "n");
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setDefault(1.0f, 1.0f, 1.0f));
	CHECK_MSTATUS(nAttr.setHidden(true));

	aTriangleNormalCamera = nAttr.createPoint("triangleNormalCamera", "tn");
	MAKE_INPUT(nAttr);
	CHECK_MSTATUS(nAttr.setDefault(1.0f, 1.0f, 1.0f));
	CHECK_MSTATUS(nAttr.setHidden(true));

	aLightDirection = nAttr.createPoint("lightDirection", "ld");
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));
	CHECK_MSTATUS(nAttr.setDefault(1.0f, 1.0f, 1.0f));

	aLightIntensity = nAttr.createColor("lightIntensity", "li");
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));
	CHECK_MSTATUS(nAttr.setDefault(1.0f, 1.0f, 1.0f));

	aLightAmbient = nAttr.create("lightAmbient", "la",
		MFnNumericData::kBoolean);
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));

	aLightDiffuse = nAttr.create("lightDiffuse", "ldf",
		MFnNumericData::kBoolean);
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));

	aLightSpecular = nAttr.create("lightSpecular", "ls",
		MFnNumericData::kBoolean);
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));

	aLightShadowFraction = nAttr.create("lightShadowFraction", "lsf",
		MFnNumericData::kFloat);
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));

	aPreShadowIntensity = nAttr.create("preShadowIntensity", "psi",
		MFnNumericData::kFloat);
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));

	aLightBlindData = nAttr.createAddr("lightBlindData", "lbld");
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(true));
	CHECK_MSTATUS(nAttr.setWritable(false));

	aLightData = lAttr.create("lightDataArray", "ltd",
		aLightDirection, aLightIntensity, aLightAmbient,
		aLightDiffuse, aLightSpecular,
		aLightShadowFraction,
		aPreShadowIntensity,
		aLightBlindData);
	CHECK_MSTATUS(lAttr.setArray(true));
	CHECK_MSTATUS(lAttr.setStorable(false));
	CHECK_MSTATUS(lAttr.setHidden(true));
	CHECK_MSTATUS(lAttr.setDefault(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true,
		false, 0.0f, 1.0f, NULL));

	// rayOrigin
	MObject RayX = nAttr.create("rayOx", "rxo", MFnNumericData::kFloat, 0.0);
	MObject RayY = nAttr.create("rayOy", "ryo", MFnNumericData::kFloat, 0.0);
	MObject RayZ = nAttr.create("rayOz", "rzo", MFnNumericData::kFloat, 0.0);
	aRayOrigin = nAttr.create("rayOrigin", "rog", RayX, RayY, RayZ);
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(false));

	// rayDirection
	RayX = nAttr.create("rayDirectionX", "rdx", MFnNumericData::kFloat, 1.0);
	RayY = nAttr.create("rayDirectionY", "rdy", MFnNumericData::kFloat, 0.0);
	RayZ = nAttr.create("rayDirectionZ", "rdz", MFnNumericData::kFloat, 0.0);
	aRayDirection = nAttr.create("rayDirection", "rad", RayX, RayY, RayZ);
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(false));

	// objectId
	aObjectId = nAttr.createAddr("objectId", "oi");
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(false));

	// raySampler
	aRaySampler = nAttr.createAddr("raySampler", "rtr");
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(false));

	// rayDepth
	aRayDepth = nAttr.create("rayDepth", "rd", MFnNumericData::kShort, 0.0);
	CHECK_MSTATUS(nAttr.setStorable(false));
	CHECK_MSTATUS(nAttr.setHidden(true));
	CHECK_MSTATUS(nAttr.setReadable(false));

	CHECK_MSTATUS(addAttribute(aTranslucenceCoeff));
	CHECK_MSTATUS(addAttribute(aDiffuseReflectivity));
	CHECK_MSTATUS(addAttribute(aColor));
	CHECK_MSTATUS(addAttribute(aIncandescence));
	CHECK_MSTATUS(addAttribute(aPointCamera));
	CHECK_MSTATUS(addAttribute(aNormalCamera));
	CHECK_MSTATUS(addAttribute(aTriangleNormalCamera));

	CHECK_MSTATUS(addAttribute(aLightData));

	CHECK_MSTATUS(addAttribute(aPower));
	CHECK_MSTATUS(addAttribute(aSpecularity));
	CHECK_MSTATUS(addAttribute(aOutColor));

	CHECK_MSTATUS(addAttribute(aRayOrigin));
	CHECK_MSTATUS(addAttribute(aRayDirection));
	CHECK_MSTATUS(addAttribute(aObjectId));
	CHECK_MSTATUS(addAttribute(aRaySampler));
	CHECK_MSTATUS(addAttribute(aRayDepth));
	CHECK_MSTATUS(addAttribute(aReflectGain));

	CHECK_MSTATUS(attributeAffects(aTranslucenceCoeff, aOutColor));
	CHECK_MSTATUS(attributeAffects(aDiffuseReflectivity, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightIntensity, aOutColor));
	CHECK_MSTATUS(attributeAffects(aIncandescence, aOutColor));
	CHECK_MSTATUS(attributeAffects(aPointCamera, aOutColor));
	CHECK_MSTATUS(attributeAffects(aNormalCamera, aOutColor));
	CHECK_MSTATUS(attributeAffects(aTriangleNormalCamera, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightData, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightAmbient, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightSpecular, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightDiffuse, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightDirection, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightShadowFraction, aOutColor));
	CHECK_MSTATUS(attributeAffects(aPreShadowIntensity, aOutColor));
	CHECK_MSTATUS(attributeAffects(aLightBlindData, aOutColor));
	CHECK_MSTATUS(attributeAffects(aPower, aOutColor));
	CHECK_MSTATUS(attributeAffects(aSpecularity, aOutColor));
	CHECK_MSTATUS(attributeAffects(aColor, aOutColor));

	CHECK_MSTATUS(attributeAffects(aRayOrigin, aOutColor));
	CHECK_MSTATUS(attributeAffects(aRayDirection, aOutColor));
	CHECK_MSTATUS(attributeAffects(aObjectId, aOutColor));
	CHECK_MSTATUS(attributeAffects(aRaySampler, aOutColor));
	CHECK_MSTATUS(attributeAffects(aRayDepth, aOutColor));
	CHECK_MSTATUS(attributeAffects(aReflectGain, aOutColor));

	return MS::kSuccess;
}


//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus ToonNode::compute(
	const MPlug&      plug,
	MDataBlock& block)
{
	if ((plug != aOutColor) && (plug.parent() != aOutColor))
		return MS::kUnknownParameter;

	MFloatVector resultColor(0.0, 0.0, 0.0);

	// get sample surface shading parameters
	MFloatVector& surfaceNormal = block.inputValue(aNormalCamera).asFloatVector();
	MFloatVector& cameraPosition = block.inputValue(aPointCamera).asFloatVector();

	// use for raytracing api enhancement below
	MFloatVector point = cameraPosition;
	MFloatVector normal = surfaceNormal;

	MFloatVector& surfaceColor = block.inputValue(aColor).asFloatVector();
	MFloatVector& incandescence = block.inputValue(aIncandescence).asFloatVector();
	float diffuseReflectivity = block.inputValue(aDiffuseReflectivity).asFloat();
	// float translucenceCoeff   = block.inputValue( aTranslucenceCoeff ).asFloat();
	// User-defined Reflection Color Gain
	float reflectGain = block.inputValue(aReflectGain).asFloat();

	// Phong shading attributes
	float power = block.inputValue(aPower).asFloat();
	float spec = block.inputValue(aSpecularity).asFloat();

	float specularR, specularG, specularB;
	float diffuseR, diffuseG, diffuseB;
	diffuseR = diffuseG = diffuseB = specularR = specularG = specularB = 0.0;

	// get light list
	MArrayDataHandle lightData = block.inputArrayValue(aLightData);
	int numLights = lightData.elementCount();

	// iterate through light list and get ambient/diffuse values
	for (int count = 1; count <= numLights; count++)
	{
		MDataHandle currentLight = lightData.inputValue();
		MFloatVector& lightIntensity = currentLight.child(aLightIntensity).asFloatVector();

		// Find the blind data
		void*& blindData = currentLight.child(aLightBlindData).asAddr();

		// find ambient component
		if (currentLight.child(aLightAmbient).asBool()) {
			diffuseR += lightIntensity[0];
			diffuseG += lightIntensity[1];
			diffuseB += lightIntensity[2];
		}

		MFloatVector& lightDirection = currentLight.child(aLightDirection).asFloatVector();

		if (blindData == NULL)
		{
			// find diffuse and specular component
			if (currentLight.child(aLightDiffuse).asBool())
			{
				float cosln = lightDirection * surfaceNormal;;
				if (cosln > 0.0f)  // calculate only if facing light
				{
					diffuseR += lightIntensity[0] * (cosln * diffuseReflectivity);
					diffuseG += lightIntensity[1] * (cosln * diffuseReflectivity);
					diffuseB += lightIntensity[2] * (cosln * diffuseReflectivity);
				}

				CHECK_MSTATUS(cameraPosition.normalize());

				if (cosln > 0.0f) // calculate only if facing light
				{
					float RV = (((2 * surfaceNormal) * cosln) - lightDirection) * cameraPosition;
					if (RV > 0.0) RV = 0.0;
					if (RV < 0.0) RV = -RV;

					if (power < 0) power = -power;

					float s = spec * powf(RV, power);

					specularR += lightIntensity[0] * s;
					specularG += lightIntensity[1] * s;
					specularB += lightIntensity[2] * s;
				}
			}
		}
		else
		{
			float cosln = MRenderUtil::diffuseReflectance(blindData, lightDirection, point, surfaceNormal, true);
			if (cosln > 0.0f)  // calculate only if facing light
			{
				diffuseR += lightIntensity[0] * (cosln * diffuseReflectivity);
				diffuseG += lightIntensity[1] * (cosln * diffuseReflectivity);
				diffuseB += lightIntensity[2] * (cosln * diffuseReflectivity);
			}

			CHECK_MSTATUS(cameraPosition.normalize());

			if (currentLight.child(aLightSpecular).asBool())
			{
				MFloatVector specLightDirection = lightDirection;
				MDataHandle directionH = block.inputValue(aRayDirection);
				MFloatVector direction = directionH.asFloatVector();
				float lightAttenuation = 1.0;

				specLightDirection = MRenderUtil::maximumSpecularReflection(blindData,
					lightDirection, point, surfaceNormal, direction);
				lightAttenuation = MRenderUtil::lightAttenuation(blindData, point, surfaceNormal, false);

				// Are we facing the light
				if (specLightDirection * surfaceNormal > 0.0f)
				{
					float power2 = block.inputValue(aPower).asFloat();
					MFloatVector rv = 2 * surfaceNormal * (surfaceNormal * direction) - direction;
					float s = spec * powf(rv * specLightDirection, power2);

					specularR += lightIntensity[0] * s * lightAttenuation;
					specularG += lightIntensity[1] * s * lightAttenuation;
					specularB += lightIntensity[2] * s * lightAttenuation;
				}
			}
		}
		if (!lightData.next()) break;
	}

	//*** Start here ***//
	// add toon shading with step function
	if		(diffuseR >= 0.60f)	diffuseR = 1.0f;
	else if (diffuseR >= 0.40f)	diffuseR = 0.60f;
	else if (diffuseR >= 0.20f) diffuseR = 0.40f;
	else						diffuseR = 0.20f;

	if		(diffuseG >= 0.60f)	diffuseG = 1.0f;
	else if (diffuseG >= 0.40f)	diffuseG = 0.60f;
	else if (diffuseG >= 0.20f) diffuseG = 0.40f;
	else						diffuseG = 0.20f;

	if		(diffuseB >= 0.60f)	diffuseB = 1.0f;
	else if (diffuseB >= 0.40f)	diffuseB = 0.60f;
	else if (diffuseB >= 0.20f) diffuseB = 0.40f;
	else						diffuseB = 0.20f;

	resultColor[0] = diffuseR * surfaceColor[0];
	resultColor[1] = diffuseG * surfaceColor[1];
	resultColor[2] = diffuseB * surfaceColor[2];

	// add the reflection color
	if (reflectGain > 0.0) {
		MStatus status;

		// required attributes for using raytracer
		// origin, direction, sampler, depth, and object id.
		//
		MDataHandle originH = block.inputValue(aRayOrigin, &status);
		MFloatVector origin = originH.asFloatVector();

		MDataHandle directionH = block.inputValue(aRayDirection, &status);
		MFloatVector direction = directionH.asFloatVector();

		MDataHandle samplerH = block.inputValue(aRaySampler, &status);
		void*& samplerPtr = samplerH.asAddr();

		MDataHandle depthH = block.inputValue(aRayDepth, &status);
		short depth = depthH.asShort();

		MDataHandle objH = block.inputValue(aObjectId, &status);
		void*& objId = objH.asAddr();

		MFloatVector reflectColor;
		MFloatVector reflectTransparency;

		MFloatVector& triangleNormal = block.inputValue(aTriangleNormalCamera).asFloatVector();

		// compute reflected ray
		MFloatVector l = -direction;
		float dot = l * normal;
		if (dot < 0.0) dot = -dot;
		MFloatVector refVector = 2 * normal * dot - l; 	// reflection ray
		float dotRef = refVector * triangleNormal;
		if (dotRef < 0.0) {
			const float s = 0.01f;
			MFloatVector mVec = refVector - dotRef * triangleNormal;
			mVec.normalize();
			refVector = mVec + s * triangleNormal;
		}
		CHECK_MSTATUS(refVector.normalize());

		status = MRenderUtil::raytrace(
			point,    	//  origin
			refVector,  //  direction
			objId,		//  object id
			samplerPtr, //  sampler info
			depth,		//  ray depth
			reflectColor,	// output color and transp
			reflectTransparency);

		
		// add silhouette edge and add in the reflection color
		if (refVector * normal < 0.33f) {
			resultColor[0] = 0;
			resultColor[1] = 0;
			resultColor[2] = 0;
		}
		else {
			resultColor[0] += reflectGain * (reflectColor[0]);
			resultColor[1] += reflectGain * (reflectColor[1]);
			resultColor[2] += reflectGain * (reflectColor[2]);
		}
		
	}
	//*** End here ***//

	// set ouput color attribute
	MDataHandle outColorHandle = block.outputValue(aOutColor);
	MFloatVector& outColor = outColorHandle.asFloatVector();
	outColor = resultColor;
	outColorHandle.setClean();

	return MS::kSuccess;
}

static const MString sRegistrantId("toonShaderPlugin");

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus initializePlugin(MObject obj)
{
	const MString UserClassify("shader/surface:drawdb/shader/surface/toonNode");

	MFnPlugin plugin(obj, PLUGIN_COMPANY, "4.5", "Any");
	CHECK_MSTATUS(plugin.registerNode("toonNode", ToonNode::id,
		ToonNode::creator, ToonNode::initialize,
		MPxNode::kDependNode, &UserClassify));

	CHECK_MSTATUS(
		MHWRender::MDrawRegistry::registerSurfaceShadingNodeOverrideCreator(
			"drawdb/shader/surface/toonNode",
			sRegistrantId,
			toonShaderOverride::creator));

	return MS::kSuccess;
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin(obj);
	CHECK_MSTATUS(plugin.deregisterNode(ToonNode::id));

	CHECK_MSTATUS(MHWRender::MDrawRegistry::deregisterSurfaceShadingNodeOverrideCreator(
		"drawdb/shader/surface/toonNode", sRegistrantId));

	return MS::kSuccess;
}
