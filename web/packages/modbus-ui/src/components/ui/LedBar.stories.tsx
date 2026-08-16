import type { Meta, StoryObj } from '@storybook/react';
import { LedBar } from './LedBar';

const meta: Meta<typeof LedBar> = {
  title: 'Components/LedBar',
  component: LedBar,
  parameters: {
    layout: 'centered',
  },
  tags: ['autodocs'],
  argTypes: {
    value: { control: 'number' },
    max: { control: 'number' },
  },
};

export default meta;

type Story = StoryObj<typeof LedBar>;

export const Default: Story = {
  args: {
    value: 50,
    max: 100,
  },
};

export const Full: Story = {
  args: {
    value: 100,
    max: 100,
  },
};

export const Overload: Story = {
  args: {
    value: 150,
    max: 100,
  },
};

export const MaxOverload: Story = {
    args: {
      value: 200,
      max: 100,
    },
  };

export const ExtremeOverload: Story = {
  args: {
    value: 250,
    max: 100,
  },
}; 