# GitHub Integration Guide

## Overview
C++ Projects Manager now includes comprehensive GitHub integration, allowing you to authenticate with GitHub, browse your repositories, create new ones, and manage your development workflow seamlessly.

## Features

### 🔐 GitHub Authentication
- Secure authentication using Personal Access Tokens
- One-time setup with persistent authentication
- Visual feedback showing authenticated user

### 📚 Repository Management
- Browse all your GitHub repositories
- Clone repositories directly into workspaces
- Create new repositories on GitHub
- Automatic workspace creation for cloned repos

### 🔧 Development Integration
- Open repositories in browser
- View GitHub issues directly
- Create GitHub releases
- Rename local repositories

## Getting Started

### 1. Create a Personal Access Token
1. Go to [GitHub Settings > Developer settings > Personal access tokens](https://github.com/settings/tokens)
2. Click "Generate new token (classic)"
3. Give it a descriptive name (e.g., "C++ Projects Manager")
4. Select the following scopes:
   - `repo` - Full control of private repositories
   - `user` - Access to user profile information
5. Click "Generate token"
6. **Important**: Copy the token immediately (you won't see it again)

### 2. Authenticate in the Application
1. Open C++ Projects Manager
2. Click **"GitHub Auth"** in the Git Versioning section
3. Paste your Personal Access Token
4. The app will verify your credentials and show your username

### 3. Browse Your Repositories
1. After authentication, click **"Browse My Repos"**
2. The app will fetch all your repositories
3. Select a repository to clone
4. Choose a destination directory
5. The repository will be cloned and added as a workspace automatically

### 4. Create New Repositories
1. Click **"Create Repo"** (requires authentication)
2. Enter repository name and optional description
3. Choose public or private visibility
4. The repository will be created on GitHub
5. Optionally clone it immediately

## Workflow Examples

### Starting a New Project
1. Click **"Create Repo"** to create a new GitHub repository
2. Clone it to your local machine
3. The workspace is automatically added and selected
4. Start coding in your favorite editor
5. Use the integrated git features for version control

### Working with Existing Projects
1. Click **"Browse My Repos"** to see all your repositories
2. Select and clone any repository
3. The workspace is automatically configured
4. Build and run your project using the integrated tools

### Contributing to Projects
1. Use **"Clone from GitHub"** for any public repository
2. Enter the repository URL (works without authentication)
3. Start contributing immediately

## Security Notes

- Your Personal Access Token is stored only in memory during the session
- No credentials are saved to disk
- The token is only used for GitHub API calls
- Always use tokens with minimal required scopes

## Troubleshooting

### Authentication Issues
- **Invalid Token**: Ensure your token has the correct scopes (`repo`, `user`)
- **Network Error**: Check your internet connection
- **API Rate Limits**: GitHub limits API calls; try again later

### Repository Operations
- **Clone Failures**: Ensure you have access to the repository and git is installed
- **Create Failures**: Check that the repository name doesn't already exist
- **Permission Issues**: Ensure your token has sufficient permissions

### Requirements
- `curl` - Required for GitHub API communication
- `git` - Required for repository operations
- Internet connection - Required for all GitHub operations

## API Usage

The application uses the GitHub REST API v3:
- Authentication: `GET /user`
- Repository listing: `GET /user/repos`
- Repository creation: `POST /user/repos`

All API calls are made using `curl` with proper authentication headers.