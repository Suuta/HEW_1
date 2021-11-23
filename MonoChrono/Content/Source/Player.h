
#pragma once
#include "conioex2.h"

struct Projectile;

enum EPlayerAnimation
{
    R_Walk,
    L_Walk,
    R_Falling,
    L_Falling,
    R_Idle,
    L_Idle,
    R_Dash,
    L_Dash,
    R_Air,
    L_Air,
    R_Attack,
    L_Attack,
};

struct Player {

    Player();
    ~Player();

    // �p�����[�^�[
    Vector2 Location;
    Vector2 Size;
    Vector2 KnockBack;
    AABB    AttackCollision;  // �U������
    AABB    CapsuleCollision; // �L�����N�^�[�̔���

    // �X�e�[�^�X
    float Speed;
    float HP;
    float JumpSpeed;
    float GravityScale;
    int   JumpCount;

    // �A�N�V�������\���ǂ���
    bool bCanJump;
    bool bCanAttack;
    bool bCanMove;

    // �v���C���[�̍s�����
    bool bIsAttacking;
    bool bIsWalking;
    bool bIsJumping;
    bool bIsDash;
    bool bIsinAir;
    bool bMoveRight;
    bool bOnFloor;

    // �_���[�W�n
    bool bRecieveDamage;
    bool bHitProjectile;
    bool bKnockback;
    bool bIsVibrating;
    bool bIsInvincible;

    // �E�B�b�`�^�C��
    bool bIsWitchTime;
    bool bIsWitchtimeCoolTime;

    // �A�j���[�V�����p�����[�^�[
    float WalkAnimationTransitionTime;
    float AirAnimationTransitionTime;
    float AttackAnimationTransitionTime;

    // �T�E���h�n���h��
    SoundHandle SE_Damage;
    SoundHandle SE_Attack;

    void Init();

    // �X�V�֐�
    void Tick(float DeltaTime, float dilation);
    void UpdatePhysics(float DeltaTime);
    void UpdateAnimimation(float DeltaTime);
    void UpdateState(float DeltaTime);
    void UpdateInput(float DeltaTime);
    void Draw();
    void HUD();

    // �X�e�[�^�X�֐�
    void Damage(float amount);
    void NotifyHit(Projectile* projectile);
    void Dead();

    // �A�N�V�����֐�
    void MoveRight(float DeltaTime);
    void MoveLeft(float DeltaTime);
    void MoveUp(float DeltaTime);

    void Move(float DeltaTime, Vector2 value);	// �R���g���[���[�ړ�

    void Jump();
    void Attack();


    // �A�j���[�V�����J�ڊǗ�
    void AnimSequence(EPlayerAnimation animation, float transitionTime, int index, float DeltaTime);

    // ���݂̃A�j���[�V����
    EPlayerAnimation CurrentAnimation;

    // ���݂̃A�j���[�V�����C���f�b�N�X
    int AnimationIndex;
    int AttackAnimIndex;

    // �A�C�h���A�j���[�V����
    conioex2::Engine::Image* R_IdleAnimation;
    conioex2::Engine::Image* L_IdleAnimation;
    // �󒆃A�j���[�V����
    conioex2::Engine::Image* R_AirAnimation[2];
    conioex2::Engine::Image* L_AirAnimation[2];

    // �_�b�V���A�j���[�V����
    conioex2::Engine::Image* R_DashAnimation;
    conioex2::Engine::Image* L_DashAnimation;
    // �ړ��A�j���[�V����
    conioex2::Engine::Image* R_WalkAnimation[7];
    conioex2::Engine::Image* L_WalkAnimation[7];

    // �U���A�j���[�V����
    conioex2::Engine::Image* R_AttackAnimation[3];
    conioex2::Engine::Image* L_AttackAnimation[3];

    // �U���g���C��
    conioex2::Engine::Image* L_attackTrail;
    conioex2::Engine::Image* R_attackTrail;
};
