# GitHub Actions Workflows

This directory contains the CI/CD pipeline configurations for LPZRobots.

## Workflows

### 🚀 CI Pipeline (`ci.yml`)
**Trigger:** Push to main/master/develop, Pull Requests

The main continuous integration pipeline that runs on every code change:
- **Lint:** Code formatting and style checks
- **Build:** Multi-platform builds (Ubuntu 22.04/24.04, macOS 13/14)
- **Test:** Unit tests with CTest
- **Sanitizers:** Memory (ASAN) and undefined behavior (UBSAN) detection
- **Coverage:** Code coverage analysis with gcov/lcov
- **Integration:** Full simulation tests
- **Performance:** Benchmark tests

### 🌙 Nightly Build (`nightly.yml`)
**Trigger:** Daily at 2 AM UTC, Manual dispatch

Comprehensive nightly testing:
- **Full Matrix Build:** All OS/compiler/build type combinations
- **Memory Leak Detection:** Valgrind memcheck and heaptrack
- **Thread Safety:** ThreadSanitizer and static analysis
- **Performance Regression:** Compare against base branch
- **Full Coverage:** Detailed coverage reports
- **Security Scan:** Static security analysis
- **Documentation:** Doxygen generation

### 📦 Release (`release.yml`)
**Trigger:** Version tags (v*), Manual dispatch

Automated release pipeline:
- **Multi-platform Packages:** Linux (x64/ARM64), macOS (x64/ARM64)
- **Package Formats:** tar.gz, deb, dmg
- **Docker Images:** Multi-arch containers
- **Source Archive:** Complete source distribution
- **Documentation:** API docs generation
- **Checksums:** SHA256 for all artifacts

## Usage

### Running CI Locally
```bash
# Install act (GitHub Actions runner)
brew install act  # macOS
# or
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | bash  # Linux

# Run CI workflow locally
act -j build

# Run specific job
act -j lint
```

### Manual Workflow Dispatch
```bash
# Trigger nightly build manually
gh workflow run nightly.yml

# Create a release
gh workflow run release.yml -f version=v1.0.0
```

### Workflow Secrets
Required secrets (set in repository settings):
- `CODECOV_TOKEN`: For coverage uploads (optional)
- `GITHUB_TOKEN`: Automatically provided

## Best Practices

1. **Keep workflows DRY:** Use composite actions for repeated steps
2. **Cache dependencies:** Speeds up builds significantly
3. **Use matrix builds:** Test multiple configurations efficiently
4. **Fail fast:** Stop early on critical failures
5. **Upload artifacts:** Always save logs and test results

## Monitoring

- **Actions Tab:** View all workflow runs in the GitHub UI
- **Status Badges:** Show current build status in README
- **Email Notifications:** Configure in GitHub settings
- **Slack/Discord:** Use marketplace actions for notifications

## Troubleshooting

### Common Issues

1. **macOS builds fail with Qt errors**
   - Ensure Qt6 path is set: `echo "$(brew --prefix qt@6)/bin" >> $GITHUB_PATH`

2. **Linux builds missing dependencies**
   - Check apt package names match the Ubuntu version

3. **Coverage upload fails**
   - CODECOV_TOKEN might be missing or expired

4. **Release artifacts too large**
   - GitHub has a 2GB limit per artifact

### Debugging

Enable debug logging:
```yaml
env:
  ACTIONS_RUNNER_DEBUG: true
  ACTIONS_STEP_DEBUG: true
```

SSH into runners (using tmate):
```yaml
- name: Setup tmate session
  uses: mxschmitt/action-tmate@v3
  if: ${{ failure() }}
```

## Contributing

When modifying workflows:
1. Test changes in a feature branch first
2. Use workflow syntax validation: `actionlint`
3. Check for security issues: `actionlint -shellcheck`
4. Document any new secrets or requirements

## Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Workflow Syntax](https://docs.github.com/en/actions/reference/workflow-syntax-for-github-actions)
- [Actions Marketplace](https://github.com/marketplace?type=actions)
- [Self-hosted Runners](https://docs.github.com/en/actions/hosting-your-own-runners)