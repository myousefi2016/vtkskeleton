// Functions
void initVTK();
void renderVTK();
void prepareMenu();
void loadVessels();
void loadFile(VesselFile type);
void setupSegmentedImagePlanes();
void addToDistance(int point[3]);
bool isSkeleton(VesselFile type);
bool isEmpty(double point[3]);

// UI functions
void setDistanceText(string text);
void resetDistancePoints();
double computeDistance(double a[3], double b[3]);
void toggleCommandsMenu();
void toggleLoading();
void toggleSegmentedTransparent();
void togglePlane(ImagePlane planeToShow);
void refreshWindow();

void KeypressCallbackFunction (vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);
