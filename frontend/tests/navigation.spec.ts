import { test, expect } from '@playwright/test';

test.describe('Dashboard Keyboard Navigation & Accessibility', () => {
  test.beforeEach(async ({ page }) => {
    await page.goto('http://localhost:5173');
    await page.waitForLoadState('networkidle');
  });

  test('user can navigate tabs using keyboard alone', async ({ page }) => {
    // Focus the first navigation button in primary navigation
    const overviewBtn = page.getByRole('button', { name: 'Overview' });
    const zoneDetailsBtn = page.getByRole('button', { name: 'Zone details' });
    const alertsBtn = page.getByRole('button', { name: /Alerts/ });
    const setupBtn = page.getByRole('button', { name: 'Setup' });
    const aboutBtn = page.getByRole('button', { name: 'About & safety' });

    // Click or press Zone details
    await zoneDetailsBtn.focus();
    await page.keyboard.press('Enter');
    await expect(page.getByText('Barrier and Alert Controls')).toBeVisible();

    // Move to Alerts
    await alertsBtn.focus();
    await page.keyboard.press('Enter');
    await expect(page.getByPlaceholder('Filter by keyword...')).toBeVisible();

    // Move to Setup
    await setupBtn.focus();
    await page.keyboard.press('Enter');
    await expect(page.getByText('Zone Dimensions and Geometry')).toBeVisible();

    // Move to About & safety
    await aboutBtn.focus();
    await page.keyboard.press('Enter');
    await expect(page.getByText('Research Prototype Notice')).toBeVisible();

    // Return to Overview
    await overviewBtn.focus();
    await page.keyboard.press('Enter');
    await expect(page.getByText('Monitored Corridor Zones')).toBeVisible();
  });

  test('confirmation dialog opens and can be cancelled via keyboard', async ({ page }) => {
    // Go to Zone details
    const zoneDetailsBtn = page.getByRole('button', { name: 'Zone details' });
    await zoneDetailsBtn.click();

    // Find and trigger the Close barrier button
    const closeBarrierBtn = page.getByRole('button', { name: /Close barrier/ });
    await closeBarrierBtn.focus();
    await page.keyboard.press('Enter');

    // Confirm dialog should be visible with role="dialog"
    const dialog = page.getByRole('dialog');
    await expect(dialog).toBeVisible();
    await expect(page.getByText('Confirm: Close Barrier')).toBeVisible();
    await expect(page.getByText(/This will rotate the barrier to 90°/)).toBeVisible();

    // Focus cancel button and press Enter
    const cancelBtn = page.getByRole('button', { name: 'Cancel' });
    await cancelBtn.focus();
    await page.keyboard.press('Enter');

    // Dialog should close
    await expect(dialog).not.toBeVisible();
  });
});

