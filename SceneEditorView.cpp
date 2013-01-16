
// SceneEditorView.cpp : implementation of the CSceneEditorView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SceneEditor.h"
#endif

#include "SceneEditorDoc.h"
#include "SceneEditorView.h"
#include "ChildSceneNodeDlg.h"
#include "EntityCreatorDlg.h"
#include "SetPositionDlg.h"
#include "Terrain.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif


// CSceneEditorView

IMPLEMENT_DYNCREATE(CSceneEditorView, CView)

BEGIN_MESSAGE_MAP(CSceneEditorView, CView)
	ON_WM_PAINT()
	ON_COMMAND(ID_EDITSCENE_SCENEMANAGER, &CSceneEditorView::OnEditSceneManager)
	ON_COMMAND(ID_EDITSCENE_ADDSCENENODE, &CSceneEditorView::OnEditsceneAddscenenode)
	ON_COMMAND(ID_EDITSCENE_ADDENTITY, &CSceneEditorView::OnEditsceneAddentity)
	ON_COMMAND(ID_EDITSCENE_DESTROYENTITIES, &CSceneEditorView::OnEditsceneDestroyentity)
	ON_COMMAND(ID_SCENE_DESTROYSCENENODE, &CSceneEditorView::OnSceneDestroyscenenode)
	ON_COMMAND(ID_SCENE_SETSCENENODEPOSTION, &CSceneEditorView::OnSceneSetscenenodepostion)

//ON_WM_CHAR()
//ON_WM_KEYDOWN()
//ON_WM_KEYUP()
ON_WM_CHAR()
ON_WM_CONTEXTMENU()
ON_WM_KEYDOWN()
ON_WM_KEYUP()
ON_WM_SYSKEYDOWN()
ON_WM_SYSKEYUP()
ON_WM_RBUTTONUP()
ON_WM_TIMER()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MBUTTONDOWN()
ON_WM_MBUTTONUP()
ON_WM_MOUSEHOVER()
ON_WM_MOUSEHWHEEL()
ON_WM_MOUSEWHEEL()
ON_WM_MOUSEMOVE()
ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

// CSceneEditorView construction/destruction

CSceneEditorView::CSceneEditorView(): m_WorkingTimer(0)
		, m_MouseNavigation(false)
{
	m_Root = NULL;
	m_SceneManager = NULL;
	m_SceneManagerDlg = NULL;
	m_Camera = NULL;

}

CSceneEditorView::~CSceneEditorView()
{

}

BOOL CSceneEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CSceneEditorView drawing

void CSceneEditorView::OnDraw(CDC* /*pDC*/)
{
	CSceneEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CSceneEditorView diagnostics

#ifdef _DEBUG
void CSceneEditorView::AssertValid() const
{
	CView::AssertValid();
}

void CSceneEditorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSceneEditorDoc* CSceneEditorView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSceneEditorDoc)));
	return (CSceneEditorDoc*)m_pDocument;
}
#endif //_DEBUG


// CSceneEditorView message handlers


void CSceneEditorView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CSceneEditorDoc *Document = GetDocument();
	CEngine *Engine = ((CSceneEditorApp*)AfxGetApp())->m_Engine;
	if (Engine == NULL)
	  return;

	m_Root = Engine->GetRoot();

	if (m_First && m_Root != NULL)
    {
        m_First = false;
        EngineSetup();
    }

	if (m_Root != NULL)
    {
		m_Root->startRendering();
	}

}

void CSceneEditorView::EngineSetup(void)
{
	Ogre::Root *Root = ((CSceneEditorApp*)AfxGetApp())->m_Engine->GetRoot();
	m_SceneManager = Root->createSceneManager(Ogre::ST_GENERIC, "SceneEditor");
 
    //
    // Create a render window
    // This window should be the current ChildView window using the externalWindowHandle
    // value pair option.
    //

    Ogre::NameValuePairList parms;
    parms["externalWindowHandle"] = Ogre::StringConverter::toString((long)m_hWnd);
    parms["vsync"] = "true";

	CRect   rect;
    GetClientRect(&rect);
	Ogre::RenderTarget *RenderWindow = Root->getRenderTarget("SceneEditor");

	if (RenderWindow == NULL)
	{
	try
	{
		m_RenderWindow = Root->createRenderWindow("SceneEditor", rect.Width(), rect.Height(), false, &parms);
	}
    catch(...)
	{
		MessageBox("Cannot initialize\nCheck that graphic-card driver is up-to-date", "Initialize Render System", MB_OK | MB_ICONSTOP);
		exit(EXIT_SUCCESS);
	}
	}
// Load resources
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    // Create the camera
	m_Camera = m_SceneManager->createCamera("Camera");
	m_Camera->setPosition(Ogre::Vector3(1683, 50, 2116));
    m_Camera->lookAt(Ogre::Vector3(0, 0, 0));
    m_Camera->setNearClipDistance(0.1);
    m_Camera->setFarClipDistance(50000);
 
    if (m_Root->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_INFINITE_FAR_PLANE))
    {
        m_Camera->setFarClipDistance(0);   // enable infinite far clip distance if we can
    }

	m_Camera->setAutoTracking(false);
	m_Camera->setFixedYawAxis(true);
	m_Camera->yaw(Ogre::Radian(0));
	m_Camera->pitch(Ogre::Radian(0));

	Ogre::SceneNode *CameraNode = NULL;
	CameraNode = m_SceneManager->getRootSceneNode()->createChildSceneNode("CameraNode");
 
	m_SceneManager->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));

	//Create viewport
	Ogre::Viewport* Viewport = NULL;
	
	if (0 == m_RenderWindow->getNumViewports())
	{
		Viewport = m_RenderWindow->addViewport(m_Camera);
		Viewport->setBackgroundColour(Ogre::ColourValue(0.0f, 0.0f, 0.0f));
	}

    // Alter the camera aspect ratio to match the viewport
    m_Camera->setAspectRatio(Ogre::Real(rect.Width()) / Ogre::Real(rect.Height()));
	
/*
	Ogre::Entity *RobotEntity = SceneManager->createEntity("Robot", "robot.mesh");
	Ogre::SceneNode *RobotNode = SceneManager->getRootSceneNode()->createChildSceneNode();
	RobotNode->attachObject(RobotEntity);

	Ogre::AxisAlignedBox Box = RobotEntity->getBoundingBox();
	Ogre::Vector3 Center = Box.getCenter();
	m_Camera->lookAt(Center);
*/
	/*
	Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
 
    Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        plane, 3000, 3000, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
 
	Ogre::Entity* entGround = m_SceneManager->createEntity("GroundEntity", "ground");
	m_SceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(entGround);

    entGround->setMaterialName("Examples/Rockwall"); 
	*/
	m_SceneManager->setSkyDome(true, "Examples/CloudySky", 5, 8);
	Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
    Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(7);
 
    Ogre::Vector3 lightdir(0.55, -0.3, 0.75);
    lightdir.normalise();
 
	Ogre::Light* light = m_SceneManager->createLight("tstLight");
    light->setType(Ogre::Light::LT_DIRECTIONAL);
    light->setDirection(lightdir);
    light->setDiffuseColour(Ogre::ColourValue::White);
    light->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));
 
	m_SceneManager->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));
	
	mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();
	mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(m_SceneManager, Ogre::Terrain::ALIGN_X_Z, 513, 12000.0f);
    mTerrainGroup->setFilenameConvention(Ogre::String("Terrain"), Ogre::String("dat"));
    mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);
 
    configureTerrainDefaults(light);
 
    for (long x = 0; x <= 0; ++x)
        for (long y = 0; y <= 0; ++y)
            defineTerrain(x, y);
 
    // sync load since we want everything in place when we start
    mTerrainGroup->loadAllTerrains(true);
 
    if (mTerrainsImported)
    {
        Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
        while(ti.hasMoreElements())
        {
            Ogre::Terrain* t = ti.getNext()->instance;
            initBlendMaps(t);
        }
    }
 
    mTerrainGroup->freeTemporaryResources();
}


void getTerrainImage(bool flipX, bool flipY, Ogre::Image& img)
{
	img.load("terrain1.bmp", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    if (flipX)
        img.flipAroundY();
    if (flipY)
        img.flipAroundX();
}
//-------------------------------------------------------------------------------------
void CSceneEditorView::defineTerrain(long x, long y)
{
	Ogre::String filename = mTerrainGroup->generateFilename(x, y);
    if (Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename))
    {
        mTerrainGroup->defineTerrain(x, y);
    }
    else
    {
        Ogre::Image img;
        getTerrainImage(x % 2 != 0, y % 2 != 0, img);
        mTerrainGroup->defineTerrain(x, y, &img);
        mTerrainsImported = true;
    } 
}
//-------------------------------------------------------------------------------------
void CSceneEditorView::initBlendMaps(Ogre::Terrain* terrain)
{
	Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
    Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
    Ogre::Real minHeight0 = 70;
    Ogre::Real fadeDist0 = 40;
    Ogre::Real minHeight1 = 70;
    Ogre::Real fadeDist1 = 15;
    float* pBlend0 = blendMap0->getBlendPointer();
    float* pBlend1 = blendMap1->getBlendPointer();
    for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
    {
        for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
        {
            Ogre::Real tx, ty;
 
            blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
            Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
            Ogre::Real val = (height - minHeight0) / fadeDist0;
            val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
            *pBlend0++ = val;
 
            val = (height - minHeight1) / fadeDist1;
            val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
            *pBlend1++ = val;
        }
    }
    blendMap0->dirty();
    blendMap1->dirty();
    blendMap0->update();
    blendMap1->update();
}
//-------------------------------------------------------------------------------------
void CSceneEditorView::configureTerrainDefaults(Ogre::Light* light)
{
	// Configure global
    mTerrainGlobals->setMaxPixelError(8);
    // testing composite map
    mTerrainGlobals->setCompositeMapDistance(3000);
 
    // Important to set these so that the terrain knows what to use for derived (non-realtime) data
    mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
	mTerrainGlobals->setCompositeMapAmbient(m_SceneManager->getAmbientLight());
    mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());
 
    // Configure default import settings for if we use imported image
    Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
    defaultimp.terrainSize = 513;
    defaultimp.worldSize = 12000.0f;
    defaultimp.inputScale = 600;
    defaultimp.minBatchSize = 33;
    defaultimp.maxBatchSize = 65;
    // textures
    defaultimp.layerList.resize(3);
    defaultimp.layerList[0].worldSize = 100;
    defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.dds");
    defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.dds");
    defaultimp.layerList[1].worldSize = 30;
    defaultimp.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.dds");
    defaultimp.layerList[1].textureNames.push_back("grass_green-01_normalheight.dds");
    defaultimp.layerList[2].worldSize = 200;
    defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.dds");
    defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.dds"); 
}

void CSceneEditorView::OnEditSceneManager()
{
	if (this->m_SceneManagerDlg == NULL)
	{
		m_SceneManagerDlg = new CSceneManagerDlg();
		m_SceneManagerDlg->Create(IDD_SCENE_MANAGER);
	}

	m_SceneManagerDlg->ShowWindow(SW_SHOW);
}

void CSceneEditorView::OnEditsceneAddscenenode()
{
	CChildSceneNodeDlg ChildSceneNodeDlg;
	
	if (IDOK == ChildSceneNodeDlg.DoModal())
	{
		HTREEITEM Selected = m_SceneManagerDlg->m_SceneTree.GetSelectedItem();
		Ogre::String name = m_SceneManagerDlg->m_SceneTree.GetItemText(Selected);
		bool entity = m_SceneManager->hasEntity(name);
		if(name == "Scene" && !entity)
		{
			m_SceneManagerDlg->m_SceneTree.InsertItem(ChildSceneNodeDlg.m_NodeName, Selected);
			m_SceneManagerDlg->m_SceneTree.Expand(Selected, TVE_EXPAND);
			m_SceneManager->getRootSceneNode()->createChildSceneNode(Ogre::String(ChildSceneNodeDlg.m_NodeName),Ogre::Vector3(ChildSceneNodeDlg.x,ChildSceneNodeDlg.y,ChildSceneNodeDlg.z));
		}
		else
		{
			MessageBox("Root node not selected!","Error",0);
		}
	}
}

void CSceneEditorView::OnEditsceneAddentity()
{
	CEntityCreatorDlg EntityCreatorDlg;

	if (IDOK == EntityCreatorDlg.DoModal())
	{
		HTREEITEM Selected = m_SceneManagerDlg->m_SceneTree.GetSelectedItem();
		Ogre::String node_name = m_SceneManagerDlg->m_SceneTree.GetItemText(Selected);
		if(m_SceneManager->hasSceneNode(node_name))
		{
			m_SceneManagerDlg->m_SceneTree.InsertItem(EntityCreatorDlg.m_EntityName, Selected);
		
			Ogre::Entity *Entity = m_SceneManager->createEntity(Ogre::String(EntityCreatorDlg.m_EntityName), Ogre::String(EntityCreatorDlg.m_MeshName));
			Ogre::SceneNode *SceneNode = m_SceneManager->getSceneNode(node_name);
			SceneNode->attachObject(Entity);
	
			m_SceneManagerDlg->m_SceneTree.Expand(Selected, TVE_EXPAND);
		}
		else
		{
			MessageBox("Scene node is not selected!","Error",0);
		}
	}
}

void CSceneEditorView::OnEditsceneDestroyentity()
{
	// TODO: Add your command handler code here
	//Get entity and its name which is to be deleted!
	HTREEITEM selected_entity = m_SceneManagerDlg->m_SceneTree.GetSelectedItem();
	Ogre::String entity_name = m_SceneManagerDlg->m_SceneTree.GetItemText(selected_entity);
	if(m_SceneManager->hasEntity(entity_name))
	{
		//delete entity
		m_SceneManager->destroyEntity(entity_name);
		//delete entity from scene manager tree view
		m_SceneManagerDlg->m_SceneTree.DeleteItem(selected_entity);
	}
	else
	{
		MessageBox("Entity not found!","Error",0);
	}
}

void CSceneEditorView::OnSceneDestroyscenenode()
{
	// TODO: Add your command handler code here
	//Get scene node and its name which is to be deleted!
	HTREEITEM selected_node = m_SceneManagerDlg->m_SceneTree.GetSelectedItem();
	Ogre::String node_name = m_SceneManagerDlg->m_SceneTree.GetItemText(selected_node);
	if(m_SceneManager->hasSceneNode(node_name))
	{
		//Destroy scene node
		m_SceneManager->destroySceneNode(node_name);
		//Delete tree view entry
		m_SceneManagerDlg->m_SceneTree.DeleteItem(selected_node);
	}
	else
	{
		MessageBox("Scene node not found!","Error",0);
	}
}

void CSceneEditorView::OnSceneSetscenenodepostion()
{
	// TODO: Add your command handler code here
	CSetPositionDlg SetPositionDlg;
	if(IDOK == SetPositionDlg.DoModal())
	{
		HTREEITEM select_node = m_SceneManagerDlg->m_SceneTree.GetSelectedItem();
		Ogre::String node_name = m_SceneManagerDlg->m_SceneTree.GetItemText(select_node);
		if(m_SceneManager->hasSceneNode(node_name))
		{
			m_SceneManager->getSceneNode(node_name)->setPosition(Ogre::Vector3(SetPositionDlg.x,SetPositionDlg.y,SetPositionDlg.z));
		}
		else
		{
			MessageBox("Scene node not found!","Error",0);
		}
	}
}

void CSceneEditorView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnChar(nChar, nRepCnt, nFlags);
}


void CSceneEditorView::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: Add your message handler code here
}

void CSceneEditorView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{	
	//KillTimer(m_WorkingTimer);
	KillTimer(m_WorkingTimer);
	m_WorkingTimer = 0;

	switch (nChar)
	{
		case VK_LEFT: //left
			m_WorkingTimer = 1;
			break;

		case VK_UP:  //up
			m_WorkingTimer = 2;
			break;

		case VK_RIGHT: //right
			m_WorkingTimer = 3;
			break;

		case VK_DOWN: //down
			m_WorkingTimer = 4;
			break;
	}

	if (m_WorkingTimer != 0)
	  SetTimer(m_WorkingTimer, 10, NULL);
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CSceneEditorView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	KillTimer(m_WorkingTimer);
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}


void CSceneEditorView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
	if (nChar == 'R')   // cycle polygon rendering mode
    {
        Ogre::PolygonMode pm;

        switch (m_Camera->getPolygonMode())
        {
        case Ogre::PM_SOLID:
            pm = Ogre::PM_WIREFRAME;
            break;
        case Ogre::PM_WIREFRAME:
            pm = Ogre::PM_POINTS;
            break;
        default:
            pm = Ogre::PM_SOLID;
        }
        m_Camera->setPolygonMode(pm);
    }

	CView::OnSysKeyDown(nChar, nRepCnt, nFlags);
}


void CSceneEditorView::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnSysKeyUp(nChar, nRepCnt, nFlags);
}


void CSceneEditorView::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CView::OnRButtonUp(nFlags, point);
}

void CSceneEditorView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	Ogre::Vector3 CameraMove;

	switch (nIDEvent)
	{
		case 1:
			CameraMove[0] = -10;
			CameraMove[1] = 0;
			CameraMove[2] = 0;
			break;

		case 2:
			CameraMove[0] = 0;
			CameraMove[1] = 10;
			CameraMove[2] = 0;
			break;

		case 3:
			CameraMove[0] = 10;
			CameraMove[1] = 0;
			CameraMove[2] = 0;
			break;

		case 4:
			CameraMove[0] = 0;
			CameraMove[1] = -10;
			CameraMove[2] = 0;
			break;
	}

	m_Camera->moveRelative(CameraMove);
	CView::OnTimer(nIDEvent);
}


void CSceneEditorView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_MousePosition = point;
	m_MouseNavigation = true;
	CView::OnLButtonDown(nFlags, point);
}


void CSceneEditorView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_MouseNavigation = false;
	CView::OnLButtonUp(nFlags, point);
}


void CSceneEditorView::OnMButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnMButtonDown(nFlags, point);
}


void CSceneEditorView::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnMButtonUp(nFlags, point);
}


void CSceneEditorView::OnMouseHover(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnMouseHover(nFlags, point);
}


void CSceneEditorView::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// This feature requires Windows Vista or greater.
	// The symbol _WIN32_WINNT must be >= 0x0600.
	// TODO: Add your message handler code here and/or call default

	CView::OnMouseHWheel(nFlags, zDelta, pt);
}


BOOL CSceneEditorView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	Ogre::Vector3 CameraMove(0.0, 0.0, 0.0);

	CameraMove[2] = 1 * zDelta;

	CEngine * Engine = ((CSceneEditorApp*)AfxGetApp())->m_Engine;
	if (Engine == NULL)
		return false;
	Ogre::Root *Root = Engine->GetRoot();
	if (m_Camera == NULL)
		return false;
	m_Camera->moveRelative(CameraMove);

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CSceneEditorView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_MouseNavigation)
	{
		Ogre::Vector3 CameraMove(0.0, 0.0, 0.0);

		CameraMove[0] = -(m_MousePosition.x - point.x);
		CameraMove[1] = -(m_MousePosition.y - point.y);

		CEngine * Engine = ((CSceneEditorApp*)AfxGetApp())->m_Engine;
		if (Engine == NULL)
			return;
		Ogre::Root *Root = Engine->GetRoot();
		if (m_Camera == NULL)
			return;
		m_Camera->yaw(Ogre::Degree(CameraMove[0] * 0.10f));
		m_Camera->pitch(Ogre::Degree(CameraMove[1] * 0.10f));

		m_MousePosition = point;
	}

	CView::OnMouseMove(nFlags, point);
}


void CSceneEditorView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CView::OnRButtonDown(nFlags, point);
}
