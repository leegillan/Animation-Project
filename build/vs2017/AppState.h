#pragma once

enum APP_STATE
{
	KEYFRAME2D,
	SKELETON2D,
	BLENDING3D,
	RAGDOLLS3D
};

enum DIMENSION
{
	Anim2D,
	Anim3D
};

class AppState
{
public:
	AppState();
	~AppState();

	void Init();

	//setters & getters
	void SetAppState(APP_STATE aS) { appState = aS; }
	APP_STATE GetAppState() { return appState; }

	void SetDimension(DIMENSION d) { dimension = d; }
	DIMENSION GetDimension() { return dimension; }

private:
	APP_STATE appState;
	DIMENSION dimension;

};

