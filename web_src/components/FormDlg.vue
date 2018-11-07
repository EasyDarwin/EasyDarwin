<template>
    <div :class="['modal', {fade: fade}]" data-backdrop="static" data-disable="false" data-keyboard="true"
         tabindex="-1">
        <div :class="['modal-dialog', size]">
            <div class="modal-content">
                <div class="modal-header">
                    <button type="button" class="close" data-dismiss="modal" aria-label="Close">
                        <span aria-hidden="true">&times;</span>
                    </button>
                    <h4 class="modal-title text-center text-success"><span>{{title}}</span></h4>
                </div>
                <form class="form-horizontal" autocomplete="off" ref="form" :enctype="enctype" spellcheck="false">
                    <div class="modal-body">
                        <slot></slot>
                    </div>
                    <div class="modal-footer">
                        <!-- <button type="button" class="btn btn-default" data-dismiss="modal">取消</button> -->
                        <button type="button" class="btn btn-primary" @click.prevent="doSubmit" :disabled="disabled">确定</button>
                    </div>
                </form>
            </div>
        </div>
    </div>
</template>

<script>
    import 'jquery-ui/ui/widgets/draggable'
    import $ from 'jquery'

    export default {
        props: {
            enctype: {
                default: ''
            },
            title: {
                default: ''
            },
            size: {
                type: String,
                default: ''
            },
            fade: {
                type: Boolean,
                default: false
            },
            disabled: {
                type: Boolean,
                default: false
            }
        },
        mounted() {
            $(this.$el).find('.modal-content').draggable({
                handle: '.modal-header',
                cancel: ".modal-title span",
                addClasses: false,
                containment: 'document',
                delay: 100,
                opacity: 0.5
            });
            $(this.$el).on("shown.bs.modal", () => {
                this.$emit("show");
            }).on("hidden.bs.modal", () => {
                this.errors.clear();
                this.$emit("hide");
            })
        },
        methods: {
            show() {
                $(this.$el).modal("show");
            },
            hide() {
                $(this.$el).modal("hide");
            },
            doSubmit() {
                this.$emit("submit");
            }
        }
    }
</script>

<style lang="less" scoped>
    label.el-switch {
        margin-bottom: -10px;
    }

    .modal-content {
        overflow: hidden;
    }
</style>



