#include "Game/PlayerCharacter.hpp"
#include "Game/PlayerAnimController.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/GameCharacter.hpp"

#include "Engine/Math/RaycastUtil.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/General/ShapeComponents/CapsuleComponent.hpp"
#include "Engine/General/ShapeComponents/SphereComponent.hpp"
#include "Engine/General/ShapeComponents/CubeComponent.hpp"
#include "Engine/General/SkeletalMeshComponent.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Animation/AnimationStateMachine.hpp"
#include "Engine/Animation/AnimationState.hpp"
#include "Engine/General/Controller.hpp"

PlayerCharacter::PlayerCharacter()
    : GameCharacter()
{
}

PlayerCharacter::PlayerCharacter( std::string name, SkeletalMesh* skeletalMesh, GameRun* game )
	: GameCharacter()
{
	if (!GetSkeletalMeshComponent() && !GetSkeletalMesh())
	{
		m_name = name;
		SetSkeletalMesh( skeletalMesh );
		GetSkeletalMeshComponent()->AttachToComponent( m_rootComponent );
	}
	GetSkeletalMeshComponent()->SetLocalScale( Vec3( 0.01f, 0.01f, 0.01f ) );

	InitializeAllCollisions();

	m_gameRef = game;

	CapsuleComponent* capsule = new CapsuleComponent();
	m_boundingCollision = capsule;
	m_boundingCollision->AttachToComponent( m_rootComponent );
	m_boundingCollision->SetCollisionChannel( PAWN );

	m_boundingCollision->SetLocalPosition( Vec3( 0.f, 0.f, capsule->GetScaledCapsuleHeight() * 0.5f ) );

	m_animController = new PlayerAnimController( this );

	m_movementSpeed = 3.f;
	m_sprintParam = 3.f;

	m_maxHealth = 100.f;
	m_maxStamina = 100.f;
	m_health = 80.f;
	m_stamina = 100.f;
}

PlayerCharacter::PlayerCharacter( std::string name, std::vector<MeshT*> meshes, Skeleton const& skeleton, std::map<std::string, Texture*> textures, GameRun* game )
    : GameCharacter( name, meshes, skeleton, textures, game )
{
	m_animController = new PlayerAnimController( this );
	GetSkeletalMeshComponent()->SetLocalScale( Vec3( 0.01f, 0.01f, 0.01f ) );

	InitializeAllCollisions();

    m_movementSpeed = 3.f;
    m_sprintParam = 3.f;
}

PlayerCharacter::~PlayerCharacter()
{
}

void PlayerCharacter::Update( float deltaSeconds )
{
    Character::Update( deltaSeconds );

	m_notUsingStaminaTimer += deltaSeconds;
	m_notTakingDamageTimer += deltaSeconds;

	if (m_controller->IsSprinting() && m_controller->IsAttemptingToMove())
	{
		m_stamina = Clamp( m_stamina - 10.f * deltaSeconds, 0.f, m_maxStamina );
		m_notUsingStaminaTimer = 0.f;
	}

	if (m_notUsingStaminaTimer >= 1.f)
	{
		m_stamina = Clamp( m_stamina + 10.f * deltaSeconds, 0.f, m_maxStamina );
	}
	if (m_notTakingDamageTimer >= 3.f && m_autoRecoverableHP > 0.f)
	{
		RecoverHP( MIN( 0.5f * deltaSeconds, m_autoRecoverableHP ) );
	}

	if (m_gameRef->m_debugdraw)
	{
		CapsuleComponent* capsuleComponent = static_cast<CapsuleComponent*>(m_boundingCollision);
		DebugAddWorldWireCapsule( true,
			GetActorWorldPosition() + capsuleComponent->GetLocalPosition() - capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius()),
			GetActorWorldPosition() + capsuleComponent->GetLocalPosition() + capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius()),
			capsuleComponent->GetScaledRadius(), m_gameRef->GetDeltaSeconds(), Rgba8::WHITE, Rgba8::WHITE );
	}

	DebugAddMessage( m_animController->GetStateMachine()->GetOngoingAnimation( 0 ).GetCurrentState()->GetStateName(), deltaSeconds, Rgba8::GREEN, Rgba8::GREEN );
}

void PlayerCharacter::Render() const
{
    Character::Render();
}

std::vector<CollisionInfo> const& PlayerCharacter::GetAllCollisionInfo()
{
	return GameRun::g_collisionComponents["Paladin"];
}

void PlayerCharacter::InitializeCollisionComponents()
{
	if (GameRun::g_collisionComponents.find( "Paladin" ) != GameRun::g_collisionComponents.end() &&
		GameRun::g_collisionComponents.at( "Paladin" ).size())
		return;

	using namespace tinyxml2;
	std::string filePath = "Data/Models/FBXs/Paladin/Xml/Collisions.xml";
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

		GameRun::g_collisionComponents["Paladin"].reserve( collisionCount );

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
				GameRun::g_collisionComponents["Paladin"].push_back( newCollision );
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
				GameRun::g_collisionComponents["Paladin"].push_back( newCollision );
				componentElem = componentElem->NextSiblingElement();
			}
		}
	}
	return;
}

void PlayerCharacter::ReceiveDamage( float value, bool blocked )
{
	if (!blocked)
	{
		float damage = value;
		m_health -= damage;
		m_autoRecoverableHP = m_health >= 0.f ? damage * 0.5f : 0.f;
		m_notTakingDamageTimer = 0.f;
	}
	else
	{
		if (m_animController->GetStateMachine()->GetOngoingAnimation( 0 ).GetCurrentState()->GetStateName() == "Blocking")
		{
			float damage = value * 0.2f;
			m_health -= damage;
			m_autoRecoverableHP = m_health >= 0.f ? damage * 0.5f : 0.f;
			m_notTakingDamageTimer = 0.f;
		}
		else
		{
			float damage = value * 0.5f;
			m_health -= damage;
			m_autoRecoverableHP = m_health >= 0.f ? damage * 0.5f : 0.f;
			m_notTakingDamageTimer = 0.f;
		}
	}
}

void PlayerCharacter::RecoverHP( float amount )
{
	float RHPFilled = MIN( m_autoRecoverableHP, amount );
	m_health += RHPFilled;
	amount -= RHPFilled;
	m_autoRecoverableHP -= RHPFilled;
	m_health = Clamp( m_health + amount, 0.f, m_maxHealth );
}

void PlayerCharacter::CameraArmCollisionCheck()
{
    if (!m_gameRef)
    {
        Character::CameraArmCollisionCheck();
    }

	m_camera.m_position = GetActorWorldPosition() + m_cameraArmPivotPos;
	Vec3 cameraArmVectorNormal = m_camera.m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetNormalized();
	Vec3 cameraArmVectorNegateNormal = cameraArmVectorNormal * -1.f;

    float newLength = m_cameraArmLength;

	for (int i = 0; i < m_gameRef->m_worldCollisionList.size(); i++)
	{
		CubeComponent* cube = dynamic_cast<CubeComponent*>(m_gameRef->m_worldCollisionList[i]);
		if (!cube) continue;

		if (cube->CollisionResultAgainst( CollisionChannel::CAMERA ) != BLOCKING)
			continue;

		RaycastResult3D result;
		if (RaycastOBB3D( result, m_camera.m_position, cameraArmVectorNegateNormal, newLength, cube->CalculateBoundsOBB3D() ))
		{
			newLength = MIN( newLength, result.m_impactDist );
		}
	}

	for (int i = 0; i < m_gameRef->m_worldCollisionList.size(); i++)
	{
		CapsuleComponent* capsule = dynamic_cast<CapsuleComponent*>(m_gameRef->m_worldCollisionList[i]);
		if (!capsule) continue;

		if (capsule->CollisionResultAgainst( CollisionChannel::CAMERA ) != BLOCKING)
			continue;

		RaycastResult3D result;
		if (RaycastCapsule3D( result, m_camera.m_position, cameraArmVectorNegateNormal, newLength, capsule->GetWorldBoneStart(), capsule->GetWorldBoneEnd(), capsule->GetScaledRadius() ))
		{
			newLength = MIN( newLength, result.m_impactDist );
		}
	}
	for (int i = 0; i < m_gameRef->m_worldCollisionList.size(); i++)
	{
		SphereComponent* sphere = dynamic_cast<SphereComponent*>(m_gameRef->m_worldCollisionList[i]);
		if (!sphere) continue;

		if (sphere->CollisionResultAgainst( CollisionChannel::CAMERA ) != BLOCKING)
			continue;

		RaycastResult3D result;
		if (RaycastSphere3D( result, m_camera.m_position, cameraArmVectorNegateNormal, newLength, sphere->GetWorldPosition(), sphere->GetScaledRadius() ))
		{
			newLength = MIN( newLength, result.m_impactDist );
		}
	}

	m_camera.m_position += cameraArmVectorNegateNormal * Clamp( newLength - 0.5f, 0.f, 999.f );
}

void PlayerCharacter::ComponentCollisionCheck()
{
	if (!m_gameRef)
        return;

	bool collisionAgainstGroundFlag = false;

	for (int i = 0; i < m_gameRef->m_worldCollisionList.size(); i++)
	{
		CapsuleComponent* capsule = dynamic_cast<CapsuleComponent*>(m_gameRef->m_worldCollisionList[i]);
		if (!capsule) continue;

		CapsuleComponent* capsuleComponent = static_cast<CapsuleComponent*>(m_boundingCollision);
		Vec3 playerCapsStart = capsuleComponent->GetWorldPosition() - capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius());
		Vec3 playerCapsEnd = capsuleComponent->GetWorldPosition() + capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius());

		Vec3 center = capsule->GetWorldPosition();
		Vec3 boneStart = capsule->GetUpVector() * capsule->GetScaledHeight() * 0.5f + center;
		Vec3 boneEnd = capsule->GetUpVector() * capsule->GetScaledHeight() * -0.5f + center;
		Vec3 mtv;
		if (DoCapsulesOverlap3D( playerCapsStart, playerCapsEnd, capsuleComponent->GetScaledRadius(), boneStart, boneEnd, capsule->GetScaledRadius(), mtv ))
		{
			SetActorWorldPosition( GetActorWorldPosition() + mtv );
			if (m_gameRef->m_worldCollisionList[i]->m_name == "Ground" || m_gameRef->m_worldCollisionList[i]->m_name == "Hondou")
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
		Vec3 playerCapsStart = capsuleComponent->GetWorldPosition() - capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius());
		Vec3 playerCapsEnd = capsuleComponent->GetWorldPosition() + capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius());

		Vec3 center = sphere->GetWorldPosition();
		float radius = sphere->GetScaledRadius();
		Vec3 mtv;
		if (DoCapsuleOverlapSphere3D( playerCapsStart, playerCapsEnd, capsuleComponent->GetScaledRadius(), center, radius, mtv ))
		{
			SetActorWorldPosition( GetActorWorldPosition() + mtv );
			if (m_gameRef->m_worldCollisionList[i]->m_name == "Ground" || m_gameRef->m_worldCollisionList[i]->m_name == "Hondou")
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
		Vec3 playerCapsStart = capsuleComponent->GetWorldPosition() - capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius());
		Vec3 playerCapsEnd = capsuleComponent->GetWorldPosition() + capsuleComponent->GetUpVector() * (capsuleComponent->GetScaledCapsuleHeight() * 0.5f - capsuleComponent->GetScaledRadius());

		Vec3 center = cube->GetWorldPosition();
		OBB3 cubeOBB = cube->CalculateBoundsOBB3D();
		Vec3 mtv;
		if (DoCapsuleOverlapOBB3D( playerCapsStart, playerCapsEnd, capsuleComponent->GetScaledRadius(), cubeOBB, mtv ))
		{
			SetActorWorldPosition( GetActorWorldPosition() + mtv );
			if (m_gameRef->m_worldCollisionList[i]->m_name == "Ground" || m_gameRef->m_worldCollisionList[i]->m_name == "Hondou")
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
		if (GameRun::g_collisionComponents.find( "Paladin" ) != GameRun::g_collisionComponents.end())
		{
			for (auto const& info : GameRun::g_collisionComponents["Paladin"])
			{
				if (!m_collisionsEnabled[info.index])
					continue;

				Rgba8 color = (info.shape == CollisionShape::CAPSULE) ? Rgba8::BLUE : Rgba8::YELLOW;
				color = (info.use == CollisionUsage::ATTACK) ? Rgba8::RED : color;

				if (info.shape == CollisionShape::CAPSULE)
				{
					CapsuleComponent capsuleComponent = CapsuleComponent::CreateCapsuleComponent( info, this );
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
