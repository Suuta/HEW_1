
#include "Player.h"
#include "Game.h"
#include "Projectile.h"


Player::Player()
	: Location(120.0f, 60.0f)
	, Size(16.0f, 24.0f)
	, KnockBack(200.f, 200.0f)
	, AttackCollision(Vector2(0.0f, 0.f), Vector2(0.0f, 0.0f))
	, CapsuleCollision(Vector2(0.0f, 0.f), Vector2(0.0f, 0.0f))
	, Speed(100.0f)
	, JumpSpeed(500.0f)
	, GravityScale(150.0f)
	, bCanJump(true)
	, bIsJumping(false)
	, bIsAttacking(false)
	, bIsWalking(false)
	, HP(5.0f)
	, WalkAnimationTransitionTime(0.1f)
	, AirAnimationTransitionTime(0.1f)
	, AttackAnimationTransitionTime(0.05f)
	, AnimationIndex(0)
	, AttackAnimIndex(1)
	, JumpCount(0)
	, CurrentAnimation(EPlayerAnimation::R_Idle)
	, bMoveRight(true)
	, bRecieveDamage(true)
	, bIsDash(false)
	, bIsinAir(true)
	, bIsWitchTime(false)
	, bCanAttack(true)
	, bCanMove(true)
	, bHitProjectile(false)
	, bIsVibrating(false)
	, bKnockback(false)
	, bOnFloor(false)
	, bIsWitchtimeCoolTime(false)
	, bIsInvincible(false)
{
}

Player::~Player()
{
}

void Player::Init()
{
#if 0
	Location = Vector2(120.0f, 60.0f);
	Speed = 100.0f;
	JumpSpeed = 500.0f;
	GravityScale = 150.0f;
	bCanJump = true;
	bIsJumping = false;
	bIsAttacking = false;
	bIsWalking = false;
	HP = 5.0f;
	AnimationIndex = 0;
	AttackAnimIndex = 1;
	JumpCount = 0;
	CurrentAnimation = EPlayerAnimation::R_Idle;
	bMoveRight = true;
	bRecieveDamage = true;
	bIsDash = false;
	bIsinAir = true;
	bIsWitchTime = false;
	bCanAttack = true;
	bCanMove = true;
	bHitProjectile = false;
	bIsVibrating = false;
	bKnockback = false;
	bOnFloor = false;
	bIsWitchtimeCoolTime = false;
	bIsInvincible = false;
#endif
}

void Player::Tick(float DeltaTime, float dilation)
{
	UpdateInput(DeltaTime);			// ���͂̍X�V
	UpdatePhysics(DeltaTime);		// ����̍X�V
	UpdateState(DeltaTime);			// �X�e�[�g�̍X�V
	UpdateAnimimation(DeltaTime);	// �A�j���[�V�����̍X�V
	Draw();							// �`��̍X�V
}

void Player::MoveUp(float DeltaTime)
{
	if (Location.Y >= 2.0f)
	{
		Location.Y += -JumpSpeed * DeltaTime;
	}
}

void Player::Move(float DeltaTime, Vector2 value)
{
	if(bCanMove)
	{
		Location.X += value.X * Speed * DeltaTime;
	}
}

// �Ō�ɉ�����邩��delete���Ȃ�����
void Player::Dead()
{
	//conioex2::StopSound(Game::GetGameInstance()->BGM_Main);
	AudioManager::StopStresm(Game::GetGameInstance()->BGM_Main);

	conioex2::StopVibrate();
	Game::GetGameInstance()->OpenLevel(ELevel::GameOver);
}

void Player::Jump()
{
	if ((bCanJump) && (JumpCount < 1))
	{
		bIsJumping = true;
		JumpCount++;
	}
}

void Player::Attack()
{
	// �����蔻��ƍU���G�t�F�N�g�𐶐�
	if(!bIsAttacking && bCanAttack)
	{
		bIsAttacking = true;
		//PlaySound(SE_Attack, false);
		//SetVolume(SE_Attack, 0.1f);

		AudioManager::SetSEVolume(SE_Attack, 0.1f);
		AudioManager::PlaySE(SE_Attack);
	}
}

void Player::UpdatePhysics(float DeltaTime)
{
	// �{�b�N�X�R���W�����̍X�V
	CapsuleCollision.Min.X = Location.X;
	CapsuleCollision.Min.Y = Location.Y;
	CapsuleCollision.Max.X = Location.X + Size.X;
	CapsuleCollision.Max.Y = Location.Y + Size.Y;

	// �U������̍X�V
	if(bMoveRight)
	{
		AttackCollision.Min.X = Location.X + Size.X;
		AttackCollision.Max.X = Location.X + Size.X + 32.f;
		AttackCollision.Min.Y = Location.Y;
		AttackCollision.Max.Y = Location.Y + Size.Y;
	}
	else
	{
		AttackCollision.Min.X = Location.X - 32.f;
		AttackCollision.Max.X = Location.X;
		AttackCollision.Min.Y = Location.Y;
		AttackCollision.Max.Y = Location.Y + 24.f;
	}

	FOR(3)
	{
		if(Game::GetGameInstance()->Floors[i]->IsOnFloor(this))
		{
			GravityScale = 0.f;
			JumpCount = 0;
			bIsinAir = false;
			bOnFloor = true;
			break;
		}
		else
		{
			if (!bIsDash) GravityScale = 150.0f;
			bOnFloor = false;
		}
	}

	// �U���̃q�b�g�����G�ɒʒm
	if (bIsAttacking)
	{
		if (conioex2::IsHit(AttackCollision, Game::GetGameInstance()->m_Enemy->Collision))
		{
			Game::GetGameInstance()->m_Enemy->NotifyHit(this, 1.f);
		}
	}

	// X��ʓ�����
	if (Location.X <= 5.0f)
	{
		Location.X = 5.0f;
	}
	if (Location.X >= conioex2::Width() - Size.X - 5.f)
	{
		Location.X = conioex2::Width() - Size.X - 5.f;
	}

	// Y��ʓ�����
	if (Location.Y <= 5.0f)
	{
		Location.Y = 5.0f;
	}
	// �[���d�͂�K��
	if (Location.Y <= conioex2::Height() - Size.Y - 5.0f)
	{
		if (!bOnFloor && !bIsAttacking)
		{
			bIsinAir = true;
			Location.Y += GravityScale * DeltaTime;
		}
	}
	else
	{
		bIsinAir = false;
		JumpCount = 0;
	}
}

void Player::AnimSequence(EPlayerAnimation animation, float transitionTime, int index, float DeltaTime)
{
	static float currentTime = 0.f;
	currentTime += 1.f * DeltaTime;
	CurrentAnimation = animation;

	if (currentTime >= transitionTime)
	{
		AnimationIndex < index ? AnimationIndex++ : AnimationIndex = 0;
		currentTime = 0.f;
	}
}

void Player::UpdateAnimimation(float DeltaTime)
{
	// �ړ���
	if (bIsWalking)
	{
		// �E�ړ���
		if (bMoveRight)
		{
			// �U����
			if (bIsAttacking)
			{
				static float transitionTime = 0.f;
				transitionTime += 1.f * DeltaTime;
				CurrentAnimation = EPlayerAnimation::R_Attack;

				// 0.1�b�o�߂����玟�̃A�j���[�V�����t���[����
				if (transitionTime >= AttackAnimationTransitionTime)
				{
					AttackAnimIndex < 2 ? AttackAnimIndex++ : AttackAnimIndex = 0;
					transitionTime = 0.f;
				}
			}
			else
			{
				// �_�b�V��
				if (bIsDash)
				{
					CurrentAnimation = EPlayerAnimation::R_Dash;
				}
				else
				{
					// �󒆂��ǂ���
					if (bIsinAir)
					{
						AnimSequence(EPlayerAnimation::R_Air, AirAnimationTransitionTime, 1, DeltaTime);
					}
					else
					{
						AnimSequence(EPlayerAnimation::R_Walk, WalkAnimationTransitionTime, 6, DeltaTime);
					}
				}
			}
		}
		// ���ړ���
		else
		{
			// �U����
			if (bIsAttacking)
			{
				static float transitionTime = 0.f;
				transitionTime += 1.f * DeltaTime;
				CurrentAnimation = EPlayerAnimation::L_Attack;

				// 0.1�b�o�߂����玟�̃A�j���[�V�����t���[����
				if (transitionTime >= AttackAnimationTransitionTime)
				{
					AttackAnimIndex < 2 ? AttackAnimIndex++ : AttackAnimIndex = 0;
					transitionTime = 0.f;
				}
			}
			else
			{
				// �_�b�V��
				if (bIsDash)
				{
					CurrentAnimation = EPlayerAnimation::L_Dash;
				}
				else
				{
					// �󒆂��ǂ���
					if (bIsinAir)
					{
						AnimSequence(EPlayerAnimation::L_Air, AirAnimationTransitionTime, 1, DeltaTime);
					}
					else
					{
						AnimSequence(EPlayerAnimation::L_Walk, WalkAnimationTransitionTime, 6, DeltaTime);
					}
				}
			}
		}
	}
	// �j���[�g����
	else
	{
		if(bMoveRight)
		{
			// �U����
			if (bIsAttacking)
			{
				static float transitionTime = 0.f;
				transitionTime += 1.f * DeltaTime;
				CurrentAnimation = EPlayerAnimation::R_Attack;

				// 0.1�b�o�߂����玟�̃A�j���[�V�����t���[����
				if (transitionTime >= AttackAnimationTransitionTime)
				{
					AttackAnimIndex < 2 ? AttackAnimIndex++ : AttackAnimIndex = 0;
					transitionTime = 0.f;
				}
			}
			else
			{
				if (bIsinAir)
				{
					AnimSequence(EPlayerAnimation::R_Air, AirAnimationTransitionTime, 1, DeltaTime);
				}
				else
				{
					CurrentAnimation = EPlayerAnimation::R_Idle;
				}
			}
		}
		else
		{
			// �U����
			if (bIsAttacking)
			{
				static float transitionTime = 0.f;
				transitionTime += 1.f * DeltaTime;
				CurrentAnimation = EPlayerAnimation::L_Attack;

				// 0.1�b�o�߂����玟�̃A�j���[�V�����t���[����
				if (transitionTime >= AttackAnimationTransitionTime)
				{
					AttackAnimIndex < 2 ? AttackAnimIndex++ : AttackAnimIndex = 0;
					transitionTime = 0.f;
				}
			}
			else
			{
				if (bIsinAir)
				{
					AnimSequence(EPlayerAnimation::L_Air, AirAnimationTransitionTime, 1, DeltaTime);
				}
				else
				{
					CurrentAnimation = EPlayerAnimation::L_Idle;
				}
			}
		}
	}
}

void Player::NotifyHit(Projectile* projectile)
{
	if (!bRecieveDamage || bIsInvincible)
	{
		bHitProjectile = true;
	}
	else
	{
		if (!bIsInvincible)
		{
			Damage(1.f);
		}

		bRecieveDamage = false;
		bKnockback = true;
		conioex2::PlayVibrate();
		//SetVolume(SE_Damage, 0.1f);
		//PlaySound(SE_Damage, false);

		AudioManager::SetSEVolume(SE_Damage, 0.1f);
		AudioManager::PlaySE(SE_Damage);
		projectile->Destroy();
		bIsVibrating = true;
		bHitProjectile = false;
	}
}

void Player::UpdateState(float DeltaTime)
{
	static float jumpTime = 0.f;
	static float invincibleTime = 0.f;
	static float dashTime = 0.f;
	static float attackTime = 0.f;
	static float witchTime = 0.f;
	static float vibrateTime = 0.f;
	static float knockbackTime = 0.f;
	static float witchtimeCoolTime = 0.f;

	// ���S����
	if(HP <= 0.f)
	{
		Dead();
	}

	// �W�����v
	if (bIsJumping)
	{
		jumpTime += 1.0f * DeltaTime;
		MoveUp(DeltaTime);
	}
	if (jumpTime >= 0.2f)
	{
		bIsJumping = false;
		jumpTime = 0.0f;
	}

	// �_�b�V��
	if (bIsDash)
	{
		bIsInvincible = true;
		bCanAttack = false;
		Speed = 300.0f;
		GravityScale = 0.0f;

		dashTime += 1.0f * DeltaTime;

		// �_�b�V�����ɒe��H����Ă�����E�B�b�`�^�C���Ɉڍs
		if (bHitProjectile && !bIsWitchTime && !bIsWitchtimeCoolTime)
		{
			bIsWitchTime = true;
			bHitProjectile = false;
			Game::GetGameInstance()->SetTimeDilation(0.1f);
			Game::GetGameInstance()->InverseGrayScale();
			// conioex2::SetPitch(0, -0.5f);
			AudioManager::SetPitch(Game::GetGameInstance()->BGM_Main, -0.5f);
		}
	}
	if (dashTime >= 0.15f)
	{
		bIsDash = false;
		bIsInvincible = false;
		bCanAttack = true;
		Speed = 100.0f;
		GravityScale = 150.0f;
		dashTime = 0.0f;
	}

	// �_���[�W�㖳�G����
	if (!bRecieveDamage)
	{
		// �݌v�͗ǂ��Ȃ����ǁA��肭�����Ă�
		// ���G����ƃE�B�b�`�^�C���ڍs����̃t���O�����L���Ă��邩��
		// ��Ƀ_�b�V������Ńq�b�g�m�F�����Ă���
		// bHitProjectile = false;
		invincibleTime += 1.0f * DeltaTime;
	}
	if (invincibleTime >= 3.0f)
	{
		invincibleTime = 0.0f;
		bRecieveDamage = true;
	}

	//char str[256];
	//sprintf(str, "invincible: %.2f  CoolTime: %.2f  Recieve: %d  Hit: %d", invincibleTime, witchtimeCoolTime, bRecieveDamage, bHitProjectile);
	//conioex2::SetTitle(str);


	// �m�b�N�o�b�N��
	if (bKnockback)
	{
		bCanAttack = false;
		bIsDash = false;
		bIsJumping = false;
		bCanMove = false;
		knockbackTime += 1.0f * DeltaTime;

		if (bMoveRight)
		{
			Location.X += -KnockBack.X * DeltaTime;
			Location.Y += -KnockBack.Y * DeltaTime;
		}
		else
		{
			Location.X += KnockBack.X * DeltaTime;
			Location.Y += -KnockBack.Y * DeltaTime;
		}
	}
	if (knockbackTime >= 0.2f)
	{
		knockbackTime = 0.f;
		bKnockback = false;
		bCanAttack = true;
		bCanMove = true;
	}


	// �_���[�W���̃o�C�u����
	if (bIsVibrating)
	{
		vibrateTime += 1.0f * DeltaTime;
	}
	if (vibrateTime >= 0.5f)
	{
		vibrateTime = 0.f;
		bIsVibrating = false;
		conioex2::StopVibrate();
	}

	// �E�B�b�`�^�C������
	if (bIsWitchTime)
	{
		bHitProjectile = false; // �ēx���肳�ꂽ���Ή�
		bCanAttack = true;
		bRecieveDamage = false;

		witchTime += 1.0f * DeltaTime;
	}
	if (witchTime >= 2.5f)
	{
		Game::GetGameInstance()->SetTimeDilation(1.0f);
		Game::GetGameInstance()->SetGrayScale();
		//conioex2::SetPitch(0, 0.0f);
		AudioManager::SetPitch(Game::GetGameInstance()->BGM_Main, 0.0f);

		bIsWitchTime = false;
		bRecieveDamage = true;
		invincibleTime = 0.f;
		bIsWitchtimeCoolTime = true;
		witchTime = 0.0f; 
	}

	// �E�B�b�`�^�C���̃N�[���^�C��
	if (bIsWitchtimeCoolTime)
	{
		witchtimeCoolTime += 1.f * DeltaTime;
	}
	if (witchtimeCoolTime >= 5.f)
	{
		bIsWitchtimeCoolTime = false;
		witchtimeCoolTime = 0.f;
	}

	// �U��
	if (bIsAttacking)
	{
		attackTime += 1.0f * DeltaTime;
		GravityScale = 0.0f;
		bCanMove = false;
		bMoveRight ?
			conioex2::DrawImageGS(R_attackTrail, Location.X + 24, Location.Y):
			conioex2::DrawImageGS(L_attackTrail, Location.X - 42, Location.Y);
	}
	if (attackTime >= 0.15f)
	{
		attackTime = 0.0f;
		GravityScale = 150.0f;
		bIsAttacking = false;
		bCanMove = true;
		AttackAnimIndex = 0;
	}

	bHitProjectile = false;
}

void Player::UpdateInput(float DeltaTime)
{
	// L�X�e�B�b�N����
	Vector2 L = conioex2::GetInputXBoxAxis(Stick::Stick_L);

	// �f�b�h�]�[���Ƃ��Đ�Βl0.15�ȉ���0�� Clamp() ����
	if (abs(L.X) <= 0.15f) L.X = 0.0f;
	if (abs(L.Y) <= 0.15f) L.Y = 0.0f;

	// �N�����v��ɓ��͔��肪����Ȃ�
	if (L.X != 0.f)
	{
		Move(DeltaTime, L);
		bIsWalking = true;

		L.X > 0 ? bMoveRight = true : bMoveRight = false;
	}
	else
	{
		bIsWalking = false;
	}

	if (conioex2::GetInputXBoxButton(Button::XBox_Button_A,          XBoxInputState::XBox_Down))	Jump();
	if (conioex2::GetInputXBoxButton(Button::XBox_Button_R_Shoulder, XBoxInputState::XBox_Down))	Attack();
	if (conioex2::GetInputXBoxButton(Button::XBox_Button_L_Shoulder, XBoxInputState::XBox_Down))	if(!bKnockback) bIsDash = true;
}

void Player::Draw()
{
	HUD();

	switch (CurrentAnimation)
	{
		case EPlayerAnimation::R_Idle:
		{
			conioex2::DrawImageGS(R_IdleAnimation, Location.X, Location.Y);
			break;
		}
		case EPlayerAnimation::L_Idle:
		{
			conioex2::DrawImageGS(L_IdleAnimation, Location.X, Location.Y);
			break;
		}
		case EPlayerAnimation::R_Walk:
		{
			conioex2::DrawImageGS(R_WalkAnimation[AnimationIndex], Location.X, Location.Y);
			break;
		}
		case EPlayerAnimation::L_Walk:
		{
			conioex2::DrawImageGS(L_WalkAnimation[AnimationIndex], Location.X, Location.Y);
			break;
		}
		case EPlayerAnimation::R_Dash:
		{
			conioex2::DrawImageGS(R_DashAnimation, Location.X, Location.Y);
			break;
		}
		case EPlayerAnimation::L_Dash:
		{
			conioex2::DrawImageGS(L_DashAnimation, Location.X, Location.Y);
			break;
		}
		case EPlayerAnimation::R_Air:
		{
			conioex2::DrawImageGS(R_AirAnimation[AnimationIndex], Location.X - 8.f, Location.Y);
			break;
		}
		case EPlayerAnimation::L_Air:
		{
			conioex2::DrawImageGS(L_AirAnimation[AnimationIndex], Location.X - 8.f, Location.Y);
			break;
		}
		case EPlayerAnimation::R_Attack:
		{
			conioex2::DrawImageGS(R_AttackAnimation[AttackAnimIndex], Location.X - 8.0f, Location.Y);
			break;
		}
		case EPlayerAnimation::L_Attack:
		{
			conioex2::DrawImageGS(L_AttackAnimation[AttackAnimIndex], Location.X - 8.0f, Location.Y);
			break;
		}
	}
}

void Player::HUD()
{
	if(HP == 1.0f)
	{
		conioex2::DrawImageGS(Game::GetGameInstance()->m_hp_1, 5, 5);
	}
	else if (HP == 2.0f)
	{
		conioex2::DrawImageGS(Game::GetGameInstance()->m_hp_2, 5, 5);
	}
	else if (HP == 3.0f)
	{
		conioex2::DrawImageGS(Game::GetGameInstance()->m_hp_3, 5, 5);
	}
	else if (HP == 4.0f)
	{
		conioex2::DrawImageGS(Game::GetGameInstance()->m_hp_4, 5, 5);
	}
	else if (HP == 5.0f)
	{
		conioex2::DrawImageGS(Game::GetGameInstance()->m_hp_5, 5, 5);
	}
}

void Player::Damage(float amount)
{
	if(HP > 0.0f)
	{
		HP -= amount;
	}
}

void Player::MoveRight(float DeltaTime)
{
	if (Location.X <= conioex2::Width() - Size.X)
	{
		Location.X += Speed * DeltaTime;
		bIsWalking = true;
		bMoveRight = true;
	}
}

void Player::MoveLeft(float DeltaTime)
{
	if (Location.X >= 1.0f)
	{
		Location.X += -Speed * DeltaTime;
		bIsWalking = true;
		bMoveRight = false;
	}
}