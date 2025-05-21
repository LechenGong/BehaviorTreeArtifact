#include "Game/Game.hpp"
#include "Game/AI/Rathian/RathianCharacter.hpp"
#include "Game/AI/Rathian/RathianAnimController.hpp"
#include "Engine/General/SkeletalMesh.hpp"
#include "Engine/General/SkeletalMeshComponent.hpp"
#include "Engine/General/ShapeComponents/CapsuleComponent.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

RathianCharacter::RathianCharacter()
    : GameCharacter()
{
}

RathianCharacter::RathianCharacter( std::string name, SkeletalMesh* skeletalMesh, GameRun* game )
	: GameCharacter()
{
	if (!GetSkeletalMesh())
	{
		m_name = name;
		SetSkeletalMesh( skeletalMesh );
		GetSkeletalMeshComponent()->AttachToComponent( m_rootComponent );
	}

	GetSkeletalMeshComponent()->SetLocalScale( Vec3( 0.01f, 0.01f, 0.01f ) );

	InitializeAllCollisions();
	
	m_gameRef = game;

	m_boundingCollision = new CapsuleComponent( 6.f, 6.f, true, PAWN );
	m_boundingCollision->AttachToComponent( m_rootComponent );
	m_boundingCollision->SetLocalScale( Vec3::ONE );

	m_animController = new RathianAnimController( this );

	m_movementSpeed = 3.f;
	m_sprintParam = 3.f;
}

RathianCharacter::RathianCharacter( std::string name, std::vector<MeshT*> meshes, Skeleton const& skeleton, std::map<std::string, Texture*> textures, GameRun* game )
	: GameCharacter()
{
	UNUSED( textures );
	m_name = name;
	SetSkeletalMesh( new SkeletalMesh( meshes, skeleton ) );
	GetSkeletalMeshComponent()->AttachToComponent( m_rootComponent );
	GetSkeletalMeshComponent()->SetLocalScale( Vec3( 0.01f, 0.01f, 0.01f ) );

	InitializeAllCollisions();

	m_gameRef = game;

	m_boundingCollision = new CapsuleComponent( 6.f, 6.f, true, PAWN );
	m_boundingCollision->AttachToComponent( m_rootComponent );
	m_boundingCollision->SetLocalScale( Vec3::ONE );
	m_boundingCollision->SetLocalPosition( Vec3( 0.f, 0.f, 3.f ) );

	m_animController = new RathianAnimController( this );

	m_movementSpeed = 3.f;
	m_sprintParam = 3.f;
}

RathianCharacter::~RathianCharacter()
{
}

void RathianCharacter::Update( float deltaSeconds )
{
	Character::Update( deltaSeconds );

	OBB3 obb = m_boundingCollision->GetBoundsOBB3D();
	Mat44 rotationMatrix = GetActorWorldOrientation().ToRotationMatrix();
	obb.m_center += GetActorWorldPosition();
	obb.m_iBasisNormal = rotationMatrix.TransformVectorQuantity3D( obb.m_iBasisNormal ).GetNormalized();
	obb.m_jBasisNormal = rotationMatrix.TransformVectorQuantity3D( obb.m_jBasisNormal ).GetNormalized();
	obb.m_kBasisNormal = rotationMatrix.TransformVectorQuantity3D( obb.m_kBasisNormal ).GetNormalized();
	DebugAddWorldWireBox( true, obb, deltaSeconds );
}

void RathianCharacter::Render() const
{
	Character::Render();
}

std::vector<CollisionInfo> const& RathianCharacter::GetAllCollisionInfo()
{
	return GameRun::g_collisionComponents["Rathian"];
}

void RathianCharacter::InitializeCollisionComponents()
{
	if (GameRun::g_collisionComponents.find( "Rathian" ) != GameRun::g_collisionComponents.end() &&
		GameRun::g_collisionComponents.at( "Rathian" ).size() )
		return;

	using namespace tinyxml2;
	std::string filePath = "Data/Models/FBXs/Rathian/Xml/Collisions.xml";
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

		GameRun::g_collisionComponents["Rathian"].reserve( collisionCount );

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

				GameRun::g_collisionComponents["Rathian"].push_back( newCollision );
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

				GameRun::g_collisionComponents["Rathian"].push_back( newCollision );
				componentElem = componentElem->NextSiblingElement();
			}
		}
	}
}
