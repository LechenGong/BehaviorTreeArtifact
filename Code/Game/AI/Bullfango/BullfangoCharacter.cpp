#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/AI/Bullfango/BullfangoCharacter.hpp"
#include "Game/AI/Bullfango/BullfangoController.hpp"
#include "Game/AI/Bullfango/BullfangoAnimController.hpp"
#include "Engine/General/SkeletalMeshComponent.hpp"
#include "Engine/General/SkeletalMesh.hpp"
#include "Engine/General/ShapeComponents/CubeComponent.hpp"
#include "Engine/General/ShapeComponents/CapsuleComponent.hpp"
#include "Engine/General/ShapeComponents/SphereComponent.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"

BullfangoCharacter::BullfangoCharacter()
    : GameCharacter()
{
}

BullfangoCharacter::BullfangoCharacter( std::string name, SkeletalMesh* skeletalMesh, GameRun* game )
	: GameCharacter()
{
	if (!GetSkeletalMesh())
	{
		m_name = name;
		SetSkeletalMesh( skeletalMesh );
		GetSkeletalMeshComponent()->AttachToComponent( m_rootComponent );
	}
	GetSkeletalMeshComponent()->SetLocalScale( Vec3( 0.01f, 0.01f, 0.01f ) );
	//GetSkeletalMeshComponent()->SetLocalPosition( Vec3( -0.2f, 0.f, 0.f ) );

	InitializeAllCollisions();

	m_gameRef = game;

	m_controller = new BullfangoController( this );

	//m_boundingCollision = new CubeComponent( 2.f, 1.f, 1.f, true, PAWN );
	m_boundingCollision = new CapsuleComponent( 0.75f, 1.25f, true, PAWN, Vec3( 0.4f, 0.f, 0.75f ), Quat( Vec3( 0.f, 1.f, 0.f ), 90.f ) );
	m_boundingCollision->AttachToComponent( m_rootComponent );
	//m_capsuleComponent->SetScale( Vec3( 4.f, 4.f, 3.4f ) );
	//m_boundingCollision->SetLocalPosition( Vec3( 0.f, 0.f, 0.5f ) );

	m_animController = new BullfangoAnimController( this );

	m_movementSpeed = 3.f;
	m_sprintParam = 3.f;

	m_maxHealth = 200.f;
	m_health = 200.f;

	g_eventSystem->SubscribeEventCallBackMethod( "BullfangoSuicide", this, &BullfangoCharacter::TestSuicide );
	//g_eventSystem->UnsubscribeEventCallbackMethod( "BullfangoSuicide", this, &BullfangoCharacter::TestSuicide );
	//g_eventSystem->UnsubscribeAllMethodsForObject( this );
}

BullfangoCharacter::BullfangoCharacter( std::string name, std::vector<MeshT*> meshes, Skeleton const& skeleton, std::map<std::string, Texture*> textures, GameRun* game )
	: GameCharacter()
{
	UNUSED( textures );
	m_name = name;
	SetSkeletalMesh( new SkeletalMesh( meshes, skeleton ) );
	GetSkeletalMeshComponent()->AttachToComponent( m_rootComponent );
	GetSkeletalMeshComponent()->SetLocalScale( Vec3( 0.01f, 0.01f, 0.01f ) );

	InitializeAllCollisions();

	m_gameRef = game;

	//m_boundingCollision = new CubeComponent( 2.f, 1.f, 1.f, true, PAWN );
	m_boundingCollision = new CapsuleComponent( 0.5f, 1.f, true, PAWN, Vec3( 0.f, 0.f, 0.5f ), Quat( Vec3( 0.f, 1.f, 0.f ), 90.f ) );
	m_boundingCollision->AttachToComponent( m_rootComponent );
	//m_capsuleComponent->SetScale( Vec3( 4.f, 4.f, 3.4f ) );
	m_boundingCollision->SetLocalPosition( Vec3( 0.f, 0.f, 0.0f ) );

	m_animController = new BullfangoAnimController( this );

	m_movementSpeed = 3.f;
	m_sprintParam = 3.f;
}

BullfangoCharacter::~BullfangoCharacter()
{
}

void BullfangoCharacter::Update( float deltaSeconds )
{
	Character::Update( deltaSeconds );

	if (m_gameRef->m_debugdraw)
	{
// 		OBB3 obb = m_boundingCollision->GetBoundsOBB3D();
// 		Mat44 rotationMatrix = GetActorWorldOrientation().ToRotationMatrix();
// 		obb.m_center += GetActorWorldPosition();
// 		obb.m_iBasisNormal = rotationMatrix.TransformVectorQuantity3D( obb.m_iBasisNormal ).GetNormalized();
// 		obb.m_jBasisNormal = rotationMatrix.TransformVectorQuantity3D( obb.m_jBasisNormal ).GetNormalized();
// 		obb.m_kBasisNormal = rotationMatrix.TransformVectorQuantity3D( obb.m_kBasisNormal ).GetNormalized();
// 		DebugAddWorldWireBox( true, obb, deltaSeconds );

		CapsuleComponent* capsuleComponent = static_cast<CapsuleComponent*>(m_boundingCollision);

		Mat44 worldTransform = GetActorWorldTransform() * capsuleComponent->GetLocalTransform();
		//Mat44 worldTransform = capsuleComponent->GetWorldTransform();
		DebugAddWorldWireCapsule( true,
			worldTransform.GetTranslation3D() - worldTransform.GetKBasis3D() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius()),
			worldTransform.GetTranslation3D() + worldTransform.GetKBasis3D() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius()),
			capsuleComponent->GetScaledRadius(), m_gameRef->GetDeltaSeconds(), Rgba8::WHITE, Rgba8::WHITE );
	}
}

void BullfangoCharacter::Render() const
{
	Character::Render();
}

 std::vector<CollisionInfo> const& BullfangoCharacter::GetAllCollisionInfo()
 {
 	return GameRun::g_collisionComponents["Bullfango"];
 }

void BullfangoCharacter::InitializeCollisionComponents()
{
	if (GameRun::g_collisionComponents.find( "Bullfango" ) != GameRun::g_collisionComponents.end() &&
		GameRun::g_collisionComponents.at( "Bullfango" ).size())
		return;

	using namespace tinyxml2;
	std::string filePath = "Data/Models/FBXs/Bullfango/Xml/Collisions.xml";
	XmlDocument doc;
	if (doc.LoadFile( filePath.c_str() ) == XML_SUCCESS)
	{
		XmlElement* root = doc.FirstChildElement( "Collisions" );

		int collisionCount = 0;
		XmlElement* bodyCollisionElem = root->FirstChildElement( "BodyCollisions" );
		XmlElement* attackCollisionElem = root->FirstChildElement( "AttackCollisions" );
		collisionCount += ParseXmlAttribute( *bodyCollisionElem, "count", 0 );
		collisionCount += ParseXmlAttribute( *attackCollisionElem, "count", 0 );
		if (collisionCount == 0)
			return;

		GameRun::g_collisionComponents["Bullfango"].reserve( collisionCount );

		if (bodyCollisionElem)
		{
			XmlElement* componentElem = bodyCollisionElem->FirstChildElement( "Component" );
			while (componentElem)
			{
				CollisionInfo newCollision;
				newCollision.index = (unsigned short)ParseXmlAttribute( *componentElem, "index", 0 );
				newCollision.shape = (CollisionShape)ParseXmlAttribute( *componentElem, "shape", 0 );
				newCollision.primaryJoint = ParseXmlAttribute( *componentElem, "primaryJoint", "" );
				newCollision.secondaryJoint = ParseXmlAttribute( *componentElem, "secondaryJoint", "" );
				newCollision.data[0] = ParseXmlAttribute( *componentElem, "x", 0.f );
				newCollision.data[1] = ParseXmlAttribute( *componentElem, "y", 0.f );
				newCollision.data[2] = ParseXmlAttribute( *componentElem, "z", 0.f );
				newCollision.data[3] = ParseXmlAttribute( *componentElem, "w", 0.f );
				newCollision.data[4] = ParseXmlAttribute( *componentElem, "x1", 0.f );
				newCollision.data[5] = ParseXmlAttribute( *componentElem, "y1", 0.f );
				newCollision.data[6] = ParseXmlAttribute( *componentElem, "z1", 0.f );
				newCollision.data[7] = ParseXmlAttribute( *componentElem, "w1", 0.f );
				newCollision.use = CollisionUsage::BODY;
				GameRun::g_collisionComponents["Bullfango"].push_back( newCollision );
				componentElem = componentElem->NextSiblingElement();
			}
		}
		if (attackCollisionElem)
		{
			XmlElement* componentElem = attackCollisionElem->FirstChildElement( "Component" );
			while (componentElem)
			{
				CollisionInfo newCollision;
				newCollision.index = (unsigned short)ParseXmlAttribute( *componentElem, "index", 0 );
				newCollision.shape = (CollisionShape)ParseXmlAttribute( *componentElem, "shape", 0 );
				newCollision.primaryJoint = ParseXmlAttribute( *componentElem, "primaryJoint", "" );
				newCollision.secondaryJoint = ParseXmlAttribute( *componentElem, "secondaryJoint", "" );
				newCollision.data[0] = ParseXmlAttribute( *componentElem, "x", 0.f );
				newCollision.data[1] = ParseXmlAttribute( *componentElem, "y", 0.f );
				newCollision.data[2] = ParseXmlAttribute( *componentElem, "z", 0.f );
				newCollision.data[3] = ParseXmlAttribute( *componentElem, "w", 0.f );
				newCollision.data[4] = ParseXmlAttribute( *componentElem, "x1", 0.f );
				newCollision.data[5] = ParseXmlAttribute( *componentElem, "y1", 0.f );
				newCollision.data[6] = ParseXmlAttribute( *componentElem, "z1", 0.f );
				newCollision.data[7] = ParseXmlAttribute( *componentElem, "w1", 0.f );
				newCollision.use = CollisionUsage::ATTACK;
				GameRun::g_collisionComponents["Bullfango"].push_back( newCollision );
				componentElem = componentElem->NextSiblingElement();
			}
		}
	}
	return;
}

void BullfangoCharacter::ComponentCollisionCheck()
{
	if (!m_gameRef)
		return;

	bool collisionAgainstGroundFlag = false;

	for (int i = 0; i < m_gameRef->m_worldCollisionList.size(); i++)
	{
		CapsuleComponent* capsule = dynamic_cast<CapsuleComponent*>(m_gameRef->m_worldCollisionList[i]);
		if (!capsule) continue;

		CapsuleComponent* capsuleComponent = static_cast<CapsuleComponent*>(m_boundingCollision);
		Vec3 bullCapsStart = capsuleComponent->GetWorldPosition() - capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius());
		Vec3 bullCapsEnd = capsuleComponent->GetWorldPosition() + capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius());

		Vec3 center = capsule->GetWorldPosition();
		Vec3 boneStart = capsule->GetUpVector() * capsule->GetScaledHeight() * 0.5f + center;
		Vec3 boneEnd = capsule->GetUpVector() * capsule->GetScaledHeight() * -0.5f + center;
		Vec3 mtv;
		if (DoCapsulesOverlap3D( bullCapsStart, bullCapsEnd, capsuleComponent->GetScaledRadius(), boneStart, boneEnd, capsule->GetScaledRadius(), mtv ))
		{
			SetActorWorldPosition( GetActorWorldPosition() + mtv );
			if (m_gameRef->m_worldCollisionList[i]->m_name == "Ground")
			{
				collisionAgainstGroundFlag = true;
			}
		}
	}

	for (int i = 0; i < m_gameRef->m_worldCollisionList.size(); i++)
	{
		SphereComponent* sphere = dynamic_cast<SphereComponent*>(m_gameRef->m_worldCollisionList[i]);
		if (!sphere) continue;

		CapsuleComponent* capsuleComponent = static_cast<CapsuleComponent*>(m_boundingCollision);
		Vec3 bullCapsStart = capsuleComponent->GetWorldPosition() - capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius());
		Vec3 bullCapsEnd = capsuleComponent->GetWorldPosition() + capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius());

		Vec3 center = sphere->GetWorldPosition();
		float radius = sphere->GetScaledRadius();
		Vec3 mtv;
		if (DoCapsuleOverlapSphere3D( bullCapsStart, bullCapsEnd, capsuleComponent->GetScaledRadius(), center, radius, mtv ))
		{
			SetActorWorldPosition( GetActorWorldPosition() + mtv );
			if (m_gameRef->m_worldCollisionList[i]->m_name == "Ground")
			{
				collisionAgainstGroundFlag = true;
			}
		}
	}
	for (int i = 0; i < m_gameRef->m_worldCollisionList.size(); i++)
	{
		CubeComponent* cube = dynamic_cast<CubeComponent*>(m_gameRef->m_worldCollisionList[i]);
		if (!cube) continue;

		CapsuleComponent* capsuleComponent = static_cast<CapsuleComponent*>(m_boundingCollision);
		Vec3 bullCapsStart = capsuleComponent->GetWorldPosition() - capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius());
		Vec3 bullCapsEnd = capsuleComponent->GetWorldPosition() + capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius());

		Vec3 center = cube->GetWorldPosition();
		OBB3 cubeOBB = cube->CalculateBoundsOBB3D();
		Vec3 mtv;
		if (DoCapsuleOverlapOBB3D( bullCapsStart, bullCapsEnd, capsuleComponent->GetScaledRadius(), cubeOBB, mtv ))
		{
			SetActorWorldPosition( GetActorWorldPosition() + mtv );
			if (m_gameRef->m_worldCollisionList[i]->m_name == "Ground")
			{
				collisionAgainstGroundFlag = true;
			}
		}
	}

	if (collisionAgainstGroundFlag)
	{
		m_isGrounded = true;
	}

	if (m_gameRef->m_debugdraw)
	{
		if (GameRun::g_collisionComponents.find( "Bullfango" ) != GameRun::g_collisionComponents.end())
		{
			for (auto const& info : GameRun::g_collisionComponents["Bullfango"])
			{
				if (!m_collisionsEnabled[info.index])
					continue;

				Rgba8 color = (info.use == CollisionUsage::BODY) ? Rgba8::BLUE : Rgba8::RED;

				if (info.shape == CollisionShape::CAPSULE)
				{
					CapsuleComponent capsuleComponent = CapsuleComponent::CreateCapsuleComponent( info, this );
					capsuleComponent.AttachToComponent( GetSkeletalMeshComponent() );
					Mat44 worldTransform = GetActorWorldTransform() * capsuleComponent.GetLocalTransform();
					DebugAddWorldWireCapsule( true,
						worldTransform.GetTranslation3D() - worldTransform.GetKBasis3D() * (capsuleComponent.GetScaledCapsuleHeight() * 0.5f - capsuleComponent.GetScaledRadius()),
						worldTransform.GetTranslation3D() + worldTransform.GetKBasis3D() * (capsuleComponent.GetScaledCapsuleHeight() * 0.5f - capsuleComponent.GetScaledRadius()),
						capsuleComponent.GetScaledRadius(), (float)App::g_actualFrameTime, color, color );
					capsuleComponent.DetachFromParent();
				}
				if (info.shape == CollisionShape::SPHERE)
				{
					SphereComponent sphereComponent = SphereComponent::CreateSphereComponent( info, this );
					sphereComponent.AttachToComponent( GetSkeletalMeshComponent() );
					Mat44 worldTransform = GetActorWorldTransform() * sphereComponent.GetLocalTransform();
					DebugAddWorldWireSphere( true,
						worldTransform.GetTranslation3D(),
						sphereComponent.GetScaledRadius(), (float)App::g_actualFrameTime, color, color );
					sphereComponent.DetachFromParent();
				}
			}
		}
	}
}

bool BullfangoCharacter::TestSuicide()
{
	m_health = 0.f;
	return false;
}
