# Unreal MCP Tools

AI-first Unreal Engine editor tooling for safe project inspection, Blueprint editing, validation, and automation.

> Active development is performed on feature branches before being merged into `main`.

## No GitHub Actions

This repository does not use GitHub Actions (owner decision, 2026-10-28).

- Don't add anything under `.github/workflows/`, and don't propose Actions-based CI/CD.
- Deployments run through the hosting platform's own Git integration: **Railway** for backends and **Cloudflare** (Workers/Pages) for frontends, building from the production branch.
- Run builds and tests locally before pushing. If a production deploy breaks, fix it and push again.
