
// �`�[����: �R�}����4
#include "Game.h"


int main(int argc, char* argv[])
{
	// COM�̏�����
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// �R���\�[���̏�����
	conioex2::InitializeConsole(300, 200, "MonoChrono", 3, 3);


	// �Q�[���̃O���[�o���C���X�^���X���擾
	Game* game = Game::GetGameInstance();

	game->Initialize();		// ������
	game->Update();			// �X�V����
	game->Finalize();		// �I������


	// �R���\�[���̏I������
	conioex2::FinalizeConsole();

	// COM�̏I������
	CoUninitialize();

	return 0;
}
