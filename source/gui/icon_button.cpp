#include "gui/icon_button.h"
#include "shader_manager.h"

IconButton::IconButton(TextureID icon, TextureID active_backgound, TextureID selected_background):
    m_icon(icon), m_active_background(active_backgound), m_selected_background(selected_background) {
	m_group = tgui::Group::create();
	m_group->setSize({ "height" , "100%" });

	m_button = tgui::BitmapButton::create();
	auto button_renderer = m_button->getRenderer();
	button_renderer->setBorders(0);
	m_button->setImage(TextureManager::Instance().textures[m_icon]);
	m_button->setImageScaling(1);
	m_button->setSize({ "100%" , "100%" });

	m_group->add(m_button);

	m_lock = tgui::Picture::create(TextureManager::Instance().textures[TextureID::Locked]);
	m_lock->setSize({ "100%" , "100%" });
	m_lock->ignoreMouseEvents(true);
	m_group->add(m_lock);
    set_state(State::Locked);
}

void IconButton::set_state(State state) {
	switch (state) {
	case IconButton::State::Locked: {
		set_grayscale();
		m_lock->setVisible(true);
		break;
	}
	case IconButton::State::Active: {
		m_lock->setVisible(false);
		tgui::Texture background(TextureManager::Instance().textures[m_active_background]);
		tgui::Texture icon(TextureManager::Instance().textures[m_icon]);
		m_button->getSharedRenderer()->setTexture(background);
		m_button->setImage(icon);
		break;
	}
	case IconButton::State::Selected: {
		m_lock->setVisible(false);
		tgui::Texture background(TextureManager::Instance().textures[m_selected_background]);
		tgui::Texture icon(TextureManager::Instance().textures[m_icon]);
		m_button->getSharedRenderer()->setTexture(background);
		m_button->setImage(icon);
		break;
	}
	case IconButton::State::Disabled:
		set_grayscale();
		break;
	default:
		break;
	}
	m_state = state;
}

void IconButton::set_grayscale() {
	tgui::Texture background(TextureManager::Instance().textures[m_active_background]);
	tgui::Texture icon(TextureManager::Instance().textures[m_icon]);
	background.setShader(&ShaderManager::Instance().shaders[Shader::GrayScale]);
	icon.setShader(&ShaderManager::Instance().shaders[Shader::GrayScale]);
	m_button->getSharedRenderer()->setTexture(background);
	m_button->setImage(icon);
}
